#include <domain/containers/removal_handler.h>
#include <domain/containers/repository.h>
#include <domain/containers/runtime.h>
#include <domain/containers/orders.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace domain::containers
{
    container_removal_handler::container_removal_handler(
        core::connections::connection &connection,
        std::shared_ptr<runtime> runtime_ptr,
        std::shared_ptr<container_repository> repository,
        const fs::path &containers_folder) : command_handler(connection),
                                             repository(repository),
                                             runtime_ptr(runtime_ptr),
                                             containers_folder(containers_folder),
                                             logger(spdlog::get("jpod"))
    {
    }
    void container_removal_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_remove_order(payload);
        if (auto result = repository->first_match(order.term); !result.has_value())
        {
            send_error(fmt::format("no container found matching: {}", order.term));
            return;
        }
        else
        {
            if (repository->is_running(result->identifier))
            {
                if (!order.force)
                {
                    send_error("container is still running, stop it to proceed with removal");
                    return;
                }
                else
                {
                    runtime_ptr->remove_container(result->identifier);
                }
            }
            if (auto error = remove_folder(result->identifier); error)
            {
                send_error(fmt::format("failed to remove container: {}", error.message()));
            }
            else
            {
                send_success(fmt::format("successfully removed container: {}", result->identifier));
            }
        }
    }
    std::error_code container_removal_handler::remove_folder(const std::string &identifier)
    {
        std::error_code error;
        auto folder = containers_folder / fs::path(identifier);
        logger->info("removed: {}", fs::remove_all(folder, error));
        return error;
    }
    void container_removal_handler::on_connection_closed(const std::error_code &error)
    {
    }
    container_removal_handler::~container_removal_handler()
    {
    }
}