#include <domain/containers/list_handler.h>
#include <domain/containers/repository.h>
#include <domain/containers/orders.h>
#include <spdlog/spdlog.h>

namespace domain::containers
{
    container_list_handler::container_list_handler(
        core::connections::connection &connection,
        std::shared_ptr<container_repository> repository) : command_handler(connection),
                                                            repository(std::move(repository)),
                                                            logger(spdlog::get("jpod"))
    {
    }

    void container_list_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_list_order(payload);
        auto entries = repository->fetch_match(order.query, mode_value(order.mode));
        send_frame(pack_container_summary(entries));
    }
    void container_list_handler::on_connection_closed(const std::error_code &error) {}
    container_list_handler::~container_list_handler() {}
}