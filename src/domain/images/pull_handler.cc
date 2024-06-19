#include <domain/images/pull_handler.h>
#include <domain/images/errors.h>
#include <domain/images/repository.h>
#include <domain/images/helpers.h>
#include <domain/images/payload.h>
#include <core/oci/oci_client.h>
#include <core/oci/payloads.h>
#include <sys/utsname.h>
#include <system_error>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

using namespace std::placeholders;

namespace domain::images
{
    pull_handler::pull_handler(
        core::connections::connection &connection,
        std::shared_ptr<image_repository> store,
        oci_client_provider provider,
        const fs::path &image_folder) : command_handler(connection),
                                        provider(provider),
                                        store(std::move(store)),
                                        image_folder(image_folder),
                                        client(nullptr),
                                        frame(std::make_unique<progress_frame>()),
                                        layer_progress{},
                                        logger(spdlog::get("jpod"))
    {
        client = provider();
    }

    void pull_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_pull_order(payload);
        if (auto result = instructions::resolve_tagged_image_details(order.target); !result)
        {
            send_error(make_error_code(error_code::invalid_order));
        }
        else if (store->has_image(result->registry, result->repository, result->tag))
        {
            send_error(make_error_code(error_code::already_exists));
        }
        else if (auto registry = store->fetch_registry_by_path(result->registry); !registry)
        {
            send_error(make_error_code(error_code::unknown_registry));
        }
        else
        {
            logger->info("GOT REG: AUTH-TYPE:{} AUTH-EP: {} REG-URL: {}", registry->authorization_type, registry->authorization_url, registry->uri);
            this->access_details.emplace(registry.value());
            this->credentials = order.credentials;
            this->repository = result->repository;
            this->tag = result->tag;
            authorize_client();
        }
    }
    void pull_handler::on_connection_closed(const std::error_code &error)
    {
    }
    void pull_handler::authorize_client()
    {
        core::oci::registry_credentials credentials{};
        credentials.name = std::string("docker");
        credentials.authorization_server = fmt::format("{}&scope=repository:{}:pull", access_details->authorization_url, repository);
        logger->info("authorization to oci registry: {}", credentials.authorization_server);
        credentials.credentials = this->credentials;
        credentials.registry = access_details->uri;
        if (auto variation = core::oci::authorization_variant_from_str(access_details->authorization_type); variation)
        {
            credentials.variant = variation.value();
            logger->info("matched for variation");
        }
        client->authorize(credentials, std::bind(&pull_handler::on_authorization, this, _1));
    }
    void pull_handler::fetch_oci_image()
    {
        image_fetch_order order{};
        utsname machine_details;

        if (uname(&machine_details) != 0)
        {
            send_error(std::error_code(errno, std::system_category()));
        }
        else
        {
            order.architecture = std::string(machine_details.machine);
            logger->info("deduced architecture: {}", order.architecture);
            order.registry = access_details->uri;
            order.repository = repository;
            order.tag = tag;
            order.operating_system = tag.find("freebsd") != std::string::npos ? "freebsd" : "linux";
            order.destination = image_folder;
            client->fetch_image(order, std::bind(&pull_handler::on_image_download, this, _1, _2, _3));
        }
    }
    void pull_handler::on_authorization(const std::error_code &error)
    {
        if (error)
        {
            logger->error("failed to authorize:{}", error.message());
            send_error(error);
        }
        else
        {
            fetch_oci_image();
        }
    }
    void pull_handler::on_image_download(const std::error_code &error, const progress_update &update, const image_properties &properties)
    {
        // this is where the details will be tracked and persisted
        // track the pair of image digest and layer digest
        if (error)
        {
            send_error(error);
        }
        else if (layer_progress.find(update.layer_digest) == layer_progress.end())
        {
            layer_progress.try_emplace(update.layer_digest, update.progress);
        }
        else
        {
            layer_progress.try_emplace(update.layer_digest, update.progress);
            frame->feed = update.feed;
            frame->percentage = update.progress;
            send_progress(pack_progress_frame(*frame));
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
                        send_error(error);
                    }
                    else
                    {
                        send_success(image_identifier);
                    }
                }
            }
        }
    }
    std::error_code pull_handler::save_image_details(const std::string identifier, const image_properties &properties)
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
        details.labels.insert(properties.labels.begin(), properties.labels.end());
        details.volumes.assign(properties.volumes.begin(), properties.volumes.end());
        details.env_vars.insert(properties.env_vars.begin(), properties.env_vars.end());
        details.exposed_ports.insert(properties.exposed_ports.begin(), properties.exposed_ports.end());
        return store->save_image_details(details);
    }
    pull_handler::~pull_handler()
    {
    }
}