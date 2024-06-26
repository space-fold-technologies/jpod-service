#include <domain/containers/start_handler.h>
#include <domain/containers/repository.h>
#include <domain/containers/container.h>
#include <domain/containers/runtime.h>
#include <domain/containers/orders.h>
#include <domain/images/mappings.h>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <cmrc/cmrc.hpp>

CMRC_DECLARE(resources);

namespace domain::containers
{
    start_handler::start_handler(
        core::connections::connection &connection,
        std::shared_ptr<container_repository> repository,
        std::shared_ptr<runtime> runtime_ptr,
        const fs::path &containers_folder) : command_handler(connection),
                                             repository(repository),
                                             runtime_ptr(runtime_ptr),
                                             containers_folder(containers_folder),
                                             logger(spdlog::get("jpod"))
    {
    }

    void start_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_term_order(payload);
        // create the operation details from querying the database
        if (auto result = repository->first_match(order.term); !result.has_value())
        {
            // have to come up with custom errors for containers
            send_error(fmt::format("no matching container for : {}", order.term));
        }
        else
        {
            operation_details details{};
            details.identifier = result->identifier;
            details.hostname = "kepler";
            details.parameters.insert(result->parameters.begin(), result->parameters.end());
            details.env_vars.insert(result->env_vars.begin(), result->env_vars.end());
            details.port_map.insert(result->port_map.begin(), result->port_map.end());
            details.entry_point.assign(result->entry_point.begin(), result->entry_point.end());
            details.command.assign(result->command.begin(), result->command.end());
            details.container_folder = containers_folder / fs::path(details.identifier);
            details.network_properties = result->network_properties;

            for (const auto &entry : (!result->mount_points.empty() ? result->mount_points : from_template(result->os)))
            {
                details.mount_points.push_back(std::move(mount_point_entry{
                    entry.filesystem,
                    details.container_folder / entry.folder,
                    entry.options,
                    entry.flags}));
                auto folder = details.container_folder / entry.folder;
            }
            runtime_ptr->create_container(std::move(details));
            send_success(fmt::format("container started: {}", result->identifier));
        }
    }
    void start_handler::on_connection_closed(const std::error_code &error)
    {
        logger->info("stopping start handler");
    }
    std::vector<images::mount_point> start_handler::from_template(const std::string& os)
    {
        auto fs = cmrc::resources::get_filesystem();
        auto file = fs.open(fmt::format("{}-template.yml", os));
        std::vector<char> content(file.begin(), file.end());
        YAML::Node parsed_content = YAML::Load(std::string(content.begin(), content.end()));
        std::vector<images::mount_point> mount_points;
        for (const auto &node : parsed_content["mount_points"])
        {
            mount_points.push_back(images::mount_point{
                node["filesystem"].as<std::string>(),
                node["folder"].as<std::string>(),
                node["options"].as<std::string>(),
                node["flags"].as<uint64_t>()});
        }
        return mount_points;
    }
    start_handler::~start_handler()
    {
    }
}