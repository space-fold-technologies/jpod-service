#include <domain/images/build_handler.h>
#include <domain/images/instructions/download_instruction.h>
#include <domain/images/instructions/copy_instruction.h>
#include <domain/images/instructions/work_dir_instruction.h>
#include <domain/images/instructions/compression_instruction.h>
#include <domain/images/instructions/run_instruction.h>
#include <domain/images/instructions/mount_instruction.h>
#include <domain/images/instructions/unmount_instruction.h>
#include <domain/images/instructions/registration_instruction.h>
#include <domain/images/instructions/cleanup_instruction.h>
#include <domain/images/instructions/build_system_resolver.h>
#include <domain/images/repository.h>
#include <domain/images/payload.h>
#include <range/v3/algorithm/for_each.hpp>
#include <asio/io_context.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <sole.hpp>

namespace domain::images
{

    build_handler::build_handler(core::connections::connection &connection,
                                 std::shared_ptr<image_repository> repository,
                                 oci_client_provider provider,
                                 asio::io_context &context) : command_handler(connection),
                                                              context(context),
                                                              provider(provider),
                                                              resolver(nullptr),
                                                              repository(std::move(repository)),
                                                              logger(spdlog::get("jpod"))
    {
    }

    void build_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_build_order(payload);
        resolver = std::make_unique<build_system_resolver>(order.current_directory, stage_names);
        setup_stages(order);
        run_stages();
    }
    void build_handler::run_stages()
    {
        if (!stages.empty())
        {
            auto stage_identifier = (*stages.begin()).first;
            run_stage(stage_identifier);
        }
        else
        {
            logger->info("finished building image");
            send_success("image build complete");
        }
    }
    void build_handler::setup_stages(const build_order &order)
    {
        // TODO: from here unpack all the stages into instructions
        ranges::for_each(
            order.stages,
            [this, &order](const stage &stage)
            {
                auto stage_identifier = sole::uuid4().str();
                current_stage_work_directories.emplace(stage_identifier, fs::path("/"));
                std::deque<task> instructions;
                std::string parent_image_order;
                ranges::for_each(
                    stage.steps,
                    [this, &instructions, &stage_identifier, &parent_image_order](const std::pair<std::string, step_type> &step)
                    {
                        switch (step.second)
                        {
                        case step_type::from:
                            instructions.push_back(create_download_instruction(stage_identifier, step.first));
                            instructions.push_back(create_mount_instruction(stage_identifier, step.first));
                            parent_image_order = step.first;
                            break;

                        case step_type::copy:
                            instructions.push_back(create_copy_instruction(stage_identifier, step.first));
                            break;

                        case step_type::work_dir:
                            instructions.push_back(create_work_dir_instruction(stage_identifier, step.first));
                            break;
                        case step_type::run:
                            instructions.push_back(create_run_instruction(stage_identifier, step.first));
                            break;
                        default:
                            break;
                        }
                    });
                resolve_stage_name(stage_identifier, parent_image_order);
                instructions.push_back(create_unmount_instruction(stage_identifier, parent_image_order));
                auto last_stage = order.stages[order.stages.size() - 1];
                if (last_stage == stage)
                {
                    instructions.push_back(create_archive_instruction(stage_identifier));
                    std::vector<std::string> identifiers;
                    for (const auto &stage : stages)
                    {
                        identifiers.push_back(stage.first);
                    }
                    identifiers.push_back(stage_identifier);
                    //instructions.push_back(create_registration_instruction(stage_identifier, order, parent_image_order));
                    instructions.push_back(create_cleanup_instruction(stage_identifier, std::move(identifiers)));
                }
                this->stages.try_emplace(std::move(stage_identifier), std::move(instructions));
            });
    }
    void build_handler::resolve_stage_name(const std::string &identifer, const std::string &order)
    {
        // // check to see if there is an image alias
        if (auto position = order.find_last_of(" AS "); position != std::string::npos)
        {
            stage_names.emplace(order.substr(position + 1), identifer);
        }
        stage_names.emplace(fmt::format("{}", stages.size() - 1), identifer);
    }
    void build_handler::run_stage(const std::string &identifier)
    {
        if (auto position = stages.find(identifier); position != stages.end())
        {
            auto batch = position->second;
            auto instruction = batch.front();
            instruction->execute();
        }
    }
    task build_handler::create_download_instruction(const std::string &stage_identifier, const std::string &order)
    {
        return std::make_shared<download_instruction>(stage_identifier, order, provider, *repository.get(), *resolver.get(), *this);
    }
    task build_handler::create_mount_instruction(const std::string &stage_identifier, const std::string &order)
    {
        return std::make_shared<mount_instruction>(stage_identifier, order, *repository.get(), *resolver.get(), *this);
    }
    task build_handler::create_copy_instruction(const std::string &stage_identifier, const std::string &order)
    {
        return std::make_shared<copy_instruction>(stage_identifier, order, *resolver.get(), *this);
    }
    task build_handler::create_work_dir_instruction(const std::string &stage_identifier, const std::string &order)
    {
        return std::make_shared<work_dir_instruction>(stage_identifier, order, current_stage_work_directories[stage_identifier], *this);
    }
    task build_handler::create_run_instruction(const std::string &stage_identifier, const std::string &order)
    {
        return std::make_shared<run_instruction>(stage_identifier, order, context, current_stage_work_directories[stage_identifier], *this);
    }
    task build_handler::create_unmount_instruction(const std::string &stage_identifier, const std::string &order)
    {
        return std::make_shared<unmount_instruction>(stage_identifier, order, *repository.get(), *resolver.get(), *this);
    }
    task build_handler::create_archive_instruction(const std::string &stage_identifier)
    {
        return std::make_shared<compression_instruction>(stage_identifier, *resolver.get(), *this);
    }
    // task build_handler::create_registration_instruction(const std::string &stage_identifier, const build_order &order, const std::string &parent_order)
    // {
    //     image_properties properties{};
    //     properties.name = order.name;
    //     properties.tag = order.tag;
    //     properties.parent_image_order = parent_order;
    //     return std::make_shared<registration_instruction>(stage_identifier, std::move(properties), *repository.get(), *this);
    // }
    task build_handler::create_cleanup_instruction(const std::string &stage_identifier, std::vector<std::string> stage_identifiers)
    {
        return std::make_shared<cleanup_instruction>(stage_identifier, stage_identifiers, *resolver.get(), *this);
    }
    void build_handler::on_connection_closed(const std::error_code &error)
    {
    }
    void build_handler::on_instruction_initialized(std::string id, std::string name)
    {
        std::error_code error;
        if (name == "FROM")
        {
            if (auto position = current_stage_work_directories.find(id); position != current_stage_work_directories.end())
            {

                position->second = resolver->destination_path(id, error);
                if (error)
                {
                    logger->info("error initializing work dir: {}", error.message());
                }
            }
        }
    }
    void build_handler::on_instruction_data_received(std::string id, const std::vector<uint8_t> &content)
    {
        send_frame(content);
    }
    void build_handler::on_instruction_complete(std::string id, std::error_code err)
    {
        if (err)
        {
            send_error(err);
        }
        else if (auto position = stages.find(id); position != stages.end())
        {
            auto batch = position->second;
            batch.pop_front();
            if (!batch.empty())
            {
                run_stage(id);
            }
            else
            {
                stages.erase(position);
            }
        }
    }
    build_handler::~build_handler()
    {
        current_stage_work_directories.clear();
        stage_names.clear();
    }
}