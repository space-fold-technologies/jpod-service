#include <domain/containers/removal_handler.h>
#include <domain/containers/repository.h>
#include <domain/containers/orders.h>
#include <spdlog/spdlog.h>

namespace domain::containers
{
    container_removal_handler::container_removal_handler(
        core::connections::connection &connection,
        std::shared_ptr<container_repository> repository) : command_handler(connection),
                                                            repository(repository),
                                                            logger(spdlog::get("jpod"))
    {
    }
    void container_removal_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_term_order(payload);
        if (repository->is_running(order.term))
        {
            send_error(std::make_error_code(std::errc::device_or_resource_busy));
        }
        else if (auto error = repository->remove(order.term); error)
        {
            send_error(error);
        }
        else
        {
            send_success("container removed");
        }
    }
    void container_removal_handler::on_connection_closed(const std::error_code &error)
    {
    }
    container_removal_handler::~container_removal_handler()
    {
    }
}