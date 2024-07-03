#include <domain/images/instructions/download_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/instructions/errors.h>
#include <domain/images/helpers.h>
#include <domain/images/payload.h>
#include <domain/images/mappings.h>
#include <domain/images/repository.h>
#include <core/oci/oci_client.h>
#include <core/oci/payloads.h>
#include <sys/utsname.h>
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <fstream>

using namespace std::placeholders;

namespace domain::images::instructions
{
    download_instruction::download_instruction(
        const std::string &identifier,
        const std::string &order,
        oci_client_provider provider,
        image_repository &repository,
        directory_resolver &resolver,
        instruction_listener &listener) : instruction("FROM", listener),
                                          identifier(identifier),
                                          order(order),
                                          provider(provider),
                                          client(nullptr),
                                          repository(repository),
                                          resolver(resolver),
                                          layer_progress{},
                                          logger(spdlog::get("jpod"))
    {
    }
    void download_instruction::execute()
    {
        if (auto result = resolve_tagged_image_details(order); !result.has_value())
        {
            listener.on_instruction_complete(identifier, make_error_code(error_code::invalid_order_issued));
        }
        else if (repository.has_image(result->registry, result->repository, result->tag))
        {
            std::error_code error;
            if (fs::path stage_path = resolver.generate_image_path(identifier, error); error)
            {
                listener.on_instruction_complete(identifier, error);
            }
            else if (auto image_identifier = repository.fetch_image_identifier(result->registry, result->repository, result->tag); image_identifier.has_value())
            {
                listener.on_instruction_initialized(identifier, name);
                resolver.extract_image(
                    identifier,
                    *image_identifier,
                    [this](std::error_code error, progress_frame &progress)
                    {
                        if (!error)
                        {
                            listener.on_instruction_data_received(identifier, pack_progress_frame(progress));
                            if (static_cast<int>(progress.percentage) == 100)
                            {
                                listener.on_instruction_complete(identifier, {});
                            }
                        }
                        else
                        {
                            listener.on_instruction_complete(identifier, error);
                        }
                    });
            }
        }
        else if (auto registry = result->registry == "local" ? repository.fetch_registry_by_name("local") : repository.fetch_registry_by_path(result->registry); registry.has_value())
        {
            fetch_oci_image(*registry, result->repository, result->tag);
        }
        else
        {
            listener.on_instruction_complete(identifier, make_error_code(error_code::no_registry_entries_found));
        }
    }

    void download_instruction::fetch_oci_image(const registry_access_details &details, const std::string &repository, const std::string &tag)
    {
        image_fetch_order order{};
        utsname machine_details;

        if (uname(&machine_details) != 0)
        {
            listener.on_instruction_complete(this->identifier, std::error_code(errno, std::system_category()));
        }
        else
        {
            order.architecture = std::string(machine_details.machine);
            order.registry = details.uri;
            order.repository = repository;
            order.tag = tag;
            order.operating_system = tag.find("freebsd") != std::string::npos ? "freebsd" : "linux";
            order.destination = resolver.image_path();
            client = provider();
            client->fetch_image(order, std::bind(&download_instruction::on_image_download, this, _1, _2, _3));
        }
    }
    void download_instruction::on_image_download(const std::error_code &error, const progress_update &update, const image_properties &properties)
    {
        // this is where the details will be tracked and persisted
        // track the pair of image digest and layer digest
        if (error)
        {
            listener.on_instruction_complete(identifier, error);
        }
        else if (layer_progress.find(update.layer_digest) == layer_progress.end())
        {
            layer_progress.try_emplace(update.layer_digest, update.progress);
        }
        else
        {
            layer_progress.try_emplace(update.layer_digest, update.progress);
            frame.feed = update.feed;
            frame.percentage = update.progress;
            listener.on_instruction_data_received(identifier, pack_progress_frame(frame));
            if (layer_progress.size() == update.total_layers)
            {
                // verify completion condition by summing up and dividing to get 100
                uint16_t sum = 0;
                for (const auto &[_, progress] : layer_progress)
                {
                    sum += progress;
                }
                if (sum == static_cast<uint16_t>(update.total_layers) * 100)
                {
                    // complete
                    // save details on image
                    std::string header("sha256:");
                    std::string image_identifier(properties.digest);
                    image_identifier.replace(0, header.size(), "");
                    if (auto error = save_image_details(image_identifier, properties); error)
                    {
                        listener.on_instruction_complete(identifier, error);
                    }
                    else
                    {
                        resolver.extract_image(
                            identifier,
                            image_identifier,
                            [this](std::error_code error, progress_frame &progress)
                            {
                                if (!error)
                                {
                                    listener.on_instruction_data_received(identifier, pack_progress_frame(progress));
                                    if (static_cast<int>(progress.percentage) == 100)
                                    {
                                        listener.on_instruction_complete(identifier, {});
                                    }
                                }
                                else
                                {
                                    listener.on_instruction_complete(identifier, error);
                                }
                            });
                    }
                }
            }
        }
    }
    std::error_code download_instruction::save_image_details(const std::string identifier, const image_properties &properties)
    {
        image_details details{};
        details.identifier = identifier;
        details.registry = properties.registry;
        details.repository = properties.repository;
        details.tag = properties.tag;
        details.size = properties.size;
        details.os = properties.os;
        details.variant = properties.variant;
        details.version = properties.version;
        return repository.save_image_details(details);
    }
    download_instruction::~download_instruction()
    {
    }
}