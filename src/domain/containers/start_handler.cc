#include <domain/containers/start_handler.h>
#include <domain/containers/repository.h>
#include <domain/containers/runtime.h>
#include <domain/containers/errors.h>
#include <domain/containers/orders.h>
#include <domain/images/mappings.h>
#include <range/v3/view/split.hpp>
#include <range/v3/range/conversion.hpp>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>
#include <cmrc/cmrc.hpp>
#include <fmt/format.h>
#include <fstream>

CMRC_DECLARE(resources);
using json = nlohmann::json;
using namespace ranges;

namespace domain::containers
{
    start_handler::start_handler(
        core::connections::connection &connection,
        std::shared_ptr<container_repository> repository,
        std::shared_ptr<runtime> runtime_ptr,
        const fs::path &containers_folder,
        const fs::path &images_folder) : command_handler(connection),
                                         repository(repository),
                                         runtime_ptr(runtime_ptr),
                                         containers_folder(containers_folder),
                                         images_folder(images_folder),
                                         logger(spdlog::get("jpod"))
    {
    }

    void start_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_term_order(payload);
        auto result = initialize(order.term, containers_folder, images_folder, repository, runtime_ptr)
                          .and_then(fetch_details)
                          .and_then(prepare_container)
                          .and_then(setup_command)
                          .and_then(start_container);
        if (!result)
        {
            logger->error("container failure: {}", result.error().message());
            send_error(fmt::format("container failure: {}", result.error().message()));
        }
        else
        {
            send_success(fmt::format("container started: {}", result.value()));
        }
    }
    void start_handler::on_connection_closed(const std::error_code &error)
    {
        logger->info("stopping start handler");
    }
    startup_result start_handler::initialize(const std::string &term,
                                             const fs::path &containers_folder,
                                             const fs::path &images_folder,
                                             std::shared_ptr<container_repository> store,
                                             std::shared_ptr<runtime> runtime_ptr)
    {
        startup_state state{};
        state.term = term;
        state.containers_folder = containers_folder;
        state.images_folder = images_folder;
        state.runtime_ptr = runtime_ptr;
        state.store = store;
        return state;
    }
    startup_result start_handler::fetch_details(startup_state state)
    {
        if (auto result = state.store->first_match(state.term); !result.has_value())
        {
            return tl::make_unexpected(make_container_failure(container_error::unknown));
        }
        else
        {
            state.image_identifier = result->image_identifier;
            state.details = {};
            state.details.hostname = result->name;
            state.details.identifier = result->identifier;
            state.details.container_folder = state.containers_folder / fs::path(result->identifier);
            state.details.network_properties = result->network_properties;
            state.os = result->os;
            state.env_vars.insert(result->env_vars.begin(), result->env_vars.end());
            state.port_map.insert(result->port_map.begin(), result->port_map.end());
            return state;
        }
    }
    startup_result start_handler::prepare_container(startup_state state)
    {
        std::ifstream file(state.images_folder / fs::path("sha256") / fs::path(state.image_identifier) / fs::path("config.json"));
        auto payload = json::parse(file);
        for (auto &[key, value] : payload["config"]["ExposedPorts"].items())
        {
            auto parts = key | views::split('/') | to<std::vector<std::string>>();
            auto port = fmt::format("{}", static_cast<uint16_t>(std::atoi(parts.at(0).c_str())));
            state.details.port_map.try_emplace(port, port);
        }
        for (const auto &env_var : payload["config"]["Env"])
        {
            std::string entry = env_var.template get<std::string>();
            auto parts = entry | views::split('=') | to<std::vector<std::string>>();
            state.details.env_vars.try_emplace(parts.at(0), parts.at(1));
        }

        for (auto &part : payload["config"]["Entrypoint"])
        {
            state.entry_point.push_back(part.template get<std::string>());
        }
        for (auto &part : payload["config"]["Cmd"])
        {
            state.command.push_back(part.template get<std::string>());
        }
        return state;
    }
    startup_result start_handler::setup_command(startup_state state)
    {
        if (!state.entry_point.empty())
        {
            state.details.command.assign(state.entry_point.begin(), state.entry_point.end());
        }
        if (!state.command.empty())
        {
            if (state.command.at(0).find("/bin/sh") == std::string::npos)
            {
                state.details.command.push_back("/bin/sh");
                state.details.command.push_back("-c");
            }
            state.details.command.insert(state.details.command.end(), state.command.begin(), state.command.end());
        }
        return state;
    }
    tl::expected<std::string, std::error_code> start_handler::start_container(startup_state state)
    {
        auto fs = cmrc::resources::get_filesystem();
        auto file = fs.open(fmt::format("configurations/{}-template.yml", state.os));
        std::vector<char> content(file.begin(), file.end());
        YAML::Node parsed_content = YAML::Load(std::string(content.begin(), content.end()));
        for (const auto &node : parsed_content["parameters"])
        {
            state.details.parameters.emplace(node.first.as<std::string>(), node.second.as<std::string>());
        }
        for (const auto &node : parsed_content["mount_points"])
        {
            state.details.mount_points.push_back(mount_point_entry{
                node["filesystem"].as<std::string>(),
                state.details.container_folder / fs::path(node["folder"].as<std::string>()),
                node["options"].as<std::string>(),
                node["flags"].as<uint64_t>()});
        }
        state.details.port_map.merge(state.port_map);
        state.details.env_vars.merge(state.env_vars);
        state.runtime_ptr->create_container(std::move(state.details));
        return state.details.identifier;
    }
    start_handler::~start_handler()
    {
    }
}