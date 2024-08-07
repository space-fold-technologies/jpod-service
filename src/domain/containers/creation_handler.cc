#include <domain/containers/creation_handler.h>
#include <domain/containers/repository.h>
#include <domain/containers/orders.h>
#include <domain/containers/errors.h>
#include <domain/images/helpers.h>
#include <core/archives/helper.h>
#include <core/archives/errors.h>
#include <fmt/format.h>
#include <sole.hpp>

namespace domain::containers
{
    std::map<std::string, std::string> extensions =
        {
            {".gz", "archive"},
            {".tar", "tar ball archive"},
            {".tar.gz", "gunzip archive"},
            {".tar.xz", "xzip arhive"}};
    creation_handler::creation_handler(
        core::connections::connection &connection,
        const fs::path &containers_folder,
        const fs::path &images_folder,
        std::shared_ptr<container_repository> repository) : command_handler(connection),
                                                            containers_folder(containers_folder),
                                                            images_folder(images_folder),
                                                            repository(std::move(repository))
    {
    }
    void creation_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto identifier = sole::uuid4().str();
        auto result = initialize_creation(identifier, repository, images_folder, containers_folder, payload)
                          .and_then(extract_layers)
                          .and_then(register_container);
        if (!result)
        {
            auto target = containers_folder / fs::path(identifier);
            std::error_code error{};
            if (auto changed = fs::remove_all(target, error); error)
            {
                send_error(fmt::format("container creation failed: {}\n{}", error.message(), result.error().message()));
            }
            else
            {
                send_error(fmt::format("container creation failed: {}", result.error().message()));
            }
        }
        else
        {
            send_success(result.value()); // send the container ID over this call as the final response
        }
    }
    creation_result creation_handler::initialize_creation(
        std::string identifier,
        std::shared_ptr<container_repository> store,
        const fs::path &images_folder,
        const fs::path &containers_folder,
        const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_creation_order(payload);
        auto target = containers_folder / fs::path(identifier);
        std::error_code error{};
        if (store->exists(order.name))
        {
            return tl::unexpected(make_container_failure(container_error::exists));
        }
        else if (auto result = images::instructions::resolve_tagged_image_details(order.tagged_image); !result)
        {
            return tl::unexpected(make_container_failure(container_error::invalid_image));
        }
        else if (auto image_identifier = store->fetch_image_identifier(result->registry, result->repository, result->tag); !image_identifier)
        {
            return tl::unexpected(make_container_failure(container_error::unknown_image));
        }
        else if (!fs::create_directories(target, error) && error)
        {
            return tl::make_unexpected(error);
        }
        else
        {
            creation_state state{};
            state.name = order.name;
            state.network_properties = order.network_properties;
            state.env_vars.insert(order.env_vars.begin(), order.env_vars.end());
            state.port_map.insert(order.port_map.begin(), order.port_map.end());
            state.container_folder = target;
            state.image_folder = images_folder;
            state.image_identifier = image_identifier.value();
            state.container_identifier = identifier;
            state.store = store;
            return state;
        }
    }
    creation_result creation_handler::extract_layers(creation_state state)
    {
        fs::path image_folder = state.image_folder / fs::path("sha256") / fs::path(state.image_identifier);
        if (auto out = core::archives::initialize_writer(); !out)
        {
            return tl::make_unexpected(out.error());
        }
        else
        {
            for (auto const &entry : fs::directory_iterator(image_folder))
            {
                if (entry.is_regular_file() && (extensions.find(entry.path().extension()) != extensions.end()))
                {
                    if (auto in = core::archives::initialize_reader(entry.path()); !in)
                    {
                        return tl::make_unexpected(in.error());
                    }
                    else if (auto error = core::archives::copy_to_destination(in.value(), out.value(), state.container_folder); error)
                    {
                        return tl::make_unexpected(error);
                    }
                }
            }
        }
        return state;
    }
    tl::expected<std::string, std::error_code> creation_handler::register_container(creation_state state)
    {
        if (auto details = state.store->fetch_image_details(state.image_identifier); !details)
        {
            return tl::make_unexpected(make_container_failure(container_error::unknown_image));
        }
        else
        {
            container_properties properties{};
            properties.identifier = state.container_identifier;
            properties.os = details->os;
            properties.name = state.name;
            properties.env_vars.insert(state.env_vars.begin(), state.env_vars.end());
            properties.port_map.insert(state.port_map.begin(), state.port_map.end());
            properties.image_identifier = details->identifier;
            properties.network_properties = state.network_properties;
            if (auto error = state.store->save(properties); error)
            {
                return tl::make_unexpected(error);
            }
        }
        return state.container_identifier;
    }

    void creation_handler::on_connection_closed(const std::error_code &error) {}

    creation_handler::~creation_handler()
    {
    }
}