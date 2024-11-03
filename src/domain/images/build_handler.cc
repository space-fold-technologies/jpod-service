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
        for(const auto& stage: order.stages)
        {
            auto stage_identifier = sole::uuid4().str();
            std::error_code error{};
            if(auto path = resolver->destination_path(stage_identifier, error); !error)
            {
                current_stage_work_directories.emplace(stage_identifier, path);
            }
            
            std::deque<task> instructions;
            std::string parent_image_order;
            int index = 0;
            add_mount_instruction(stage_identifier);
            for(const auto &[step, type]: stage.steps)
            {
                        switch (type)
                            {
                            case step_type::from:
                                add_download_instruction(stage_identifier, step);
                                parent_image_order = step;
                                break;

                            case step_type::copy:
                                add_copy_instruction(stage_identifier, step);
                                break;

                            case step_type::work_dir:
                                add_work_dir_instruction(stage_identifier, step);
                                break;
                            case step_type::run:
                                add_run_instruction(stage_identifier, step);
                                break;
                            default:
                                break;
                            }
                    }
                    if(parent_image_order.empty())
                    {
                        parent_image_order = fmt::format("{}", stages.size() - 1);
                    }
                    resolve_stage_name(stage_identifier, index, parent_image_order);
                    add_unmount_instruction(stage_identifier);
                    auto last_stage = order.stages[order.stages.size() - 1];
                    if (last_stage == stage)
                    {
                        add_archive_instruction(stage_identifier);
                        std::vector<std::string> identifiers;
                        for (const auto &stage : stages)
                        {
                            identifiers.push_back(stage.first);
                        }
                        identifiers.push_back(stage_identifier);
                        //instructions.push_back(create_registration_instruction(stage_identifier, order, parent_image_order));
                        add_cleanup_instruction(stage_identifier, std::move(identifiers));
                    }
                    this->stages.try_emplace(std::move(stage_identifier), std::move(instructions));
                    index++;
        }
    }
    void build_handler::resolve_stage_name(const std::string &identifer, int index, const std::string &order)
    {
        // // check to see if there is an image alias
        if (auto position = order.find_last_of(" AS "); position != std::string::npos)
        {
            stage_names.try_emplace(order.substr(position + 1), identifer);
        } else {
            stage_names.try_emplace(fmt::format("{}", index), identifer);
        }
    }
    void build_handler::run_stage(const std::string &identifier)
    {
        stages[identifier].front()->execute();
    }
    void build_handler::add_download_instruction(const std::string &stage_identifier, const std::string &order)
    {
         stages[stage_identifier].push_back(std::move(std::make_unique<download_instruction>(stage_identifier, order, provider, *repository.get(), *resolver.get(), *this)));
    }
    void build_handler::add_mount_instruction(const std::string &stage_identifier)
    {
         stages[stage_identifier].push_back(std::move(std::make_unique<mount_instruction>(stage_identifier, *resolver.get(), *this)));
    }
    void build_handler::add_copy_instruction(const std::string &stage_identifier, const std::string &order)
    {
         stages[stage_identifier].push_back(std::move(std::make_unique<copy_instruction>(stage_identifier, order, *resolver.get(), *this)));
    }
    void build_handler::add_work_dir_instruction(const std::string &stage_identifier, const std::string &order)
    {
         stages[stage_identifier].push_back(std::move(std::make_unique<work_dir_instruction>(stage_identifier, order, current_stage_work_directories[stage_identifier], *this)));
    }
    void build_handler::add_run_instruction(const std::string &stage_identifier, const std::string &order)
    {
         stages[stage_identifier].push_back(std::move(std::make_unique<run_instruction>(stage_identifier, order, context, current_stage_work_directories[stage_identifier], *this)));
    }
    void build_handler::add_unmount_instruction(const std::string &stage_identifier)
    {
         stages[stage_identifier].push_back(std::move(std::make_unique<unmount_instruction>(stage_identifier, *resolver.get(), *this)));
    }
    void build_handler::add_archive_instruction(const std::string &stage_identifier)
    {
         stages[stage_identifier].push_back(std::move(std::make_unique<compression_instruction>(stage_identifier, *resolver.get(), *this)));
    }
    // task build_handler::create_registration_instruction(const std::string &stage_identifier, const build_order &order, const std::string &parent_order)
    // {
    //     image_properties properties{};
    //     properties.name = order.name;
    //     properties.tag = order.tag;
    //     properties.parent_image_order = parent_order;
    //     return std::make_shared<registration_instruction>(stage_identifier, std::move(properties), *repository.get(), *this);
    // }
    void build_handler::add_cleanup_instruction(const std::string &stage_identifier, std::vector<std::string> stage_identifiers)
    {
         stages[stage_identifier].push_back(std::move(std::make_unique<cleanup_instruction>(stage_identifier, stage_identifiers, *resolver.get(), *this)));
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
        } else if(current_stage_work_directories.find(id) == current_stage_work_directories.end()) {
            if(auto path = resolver->destination_path(id, error); !error)
            {
                current_stage_work_directories.try_emplace(id, path);
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
        else 
        {
            
            stages[id].pop_front();            
            if (!stages[id].empty())
            {
                run_stage(id);
            }
            else
            {
                stages.erase(stages.find(id));
                run_stages();
            }
        }
    }
    build_handler::~build_handler()
    {
        current_stage_work_directories.clear();
        stage_names.clear();
    }
}