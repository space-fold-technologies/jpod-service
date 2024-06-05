#include <domain/networking/create_handler.h>
#include <spdlog/spdlog.h>
#include <domain/networking/network_service.h>
#include <domain/networking/details.h>
#include <domain/networking/payloads.h>
#include <fmt/format.h>

namespace domain::networking
{
    create_handler::create_handler(
        core::connections::connection &connection,
        std::shared_ptr<network_service> service) : command_handler(connection),
                                                    service(std::move(service)),
                                                    logger(spdlog::get("jpod")) {}
    void create_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_network_creation_order(payload);
        network_entry entry{order.name, fmt::format("jc{}", service->total_networks()), order.subnet, order.scope, "bridge"};
        if (auto error = service->add(entry); !error)
        {
            send_success("bridge network added");
        }
        else
        {
            send_error(fmt::format("failed to add network: {}", error.message()));
            logger->error("{}", error.message());
        }
    }
    void create_handler::on_connection_closed(const std::error_code &error) {}
    create_handler::~create_handler() {}
}