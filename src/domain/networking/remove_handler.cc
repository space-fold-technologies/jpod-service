
#include <domain/networking/remove_handler.h>
#include <spdlog/spdlog.h>
#include <domain/networking/network_service.h>
#include <domain/networking/details.h>
#include <domain/networking/payloads.h>
#include <fmt/format.h>

namespace domain::networking
{
    remove_handler::remove_handler(
        core::connections::connection &connection,
        std::shared_ptr<network_service> service) : command_handler(connection),
                                                    service(std::move(service)),
                                                    logger(spdlog::get("jpod")) {}
    void remove_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_network_term_order(payload);
        if (auto error = service->remove(order.name); !error)
        {
            send_success(fmt::format("removed network: {}", order.name));
        }
        else
        {
            send_error(fmt::format("failed to remove network: {} err: {}", order.name, error.message()));
            logger->error("network removal error: {}", error.message());
        }
    }
    void remove_handler::on_connection_closed(const std::error_code &error) {}
    remove_handler::~remove_handler() {}
}