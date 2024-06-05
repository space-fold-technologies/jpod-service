
#include <domain/networking/list_handler.h>
#include <spdlog/spdlog.h>
#include <domain/networking/network_service.h>
#include <domain/networking/details.h>
#include <domain/networking/payloads.h>
#include <fmt/format.h>

namespace domain::networking
{
    list_handler::list_handler(
        core::connections::connection &connection,
        std::shared_ptr<network_service> service) : command_handler(connection),
                                                    service(std::move(service)),
                                                    logger(spdlog::get("jpod")) {}
    void list_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_network_term_order(payload);
        auto networks = service->list(order.name);
        network_list list{networks};
        auto content = pack_network_list(list);
        send_frame(content);
    }
    void list_handler::on_connection_closed(const std::error_code &error) {}
    list_handler::~list_handler() {}
}