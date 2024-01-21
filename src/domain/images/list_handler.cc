#include <domain/images/list_handler.h>
#include <domain/images/payload.h>
#include <spdlog/spdlog.h>
#include <sole.hpp>
#include <range/v3/algorithm/for_each.hpp>

using namespace std::chrono;

namespace domain::images
{
    list_handler::list_handler(core::connections::connection &connection) : command_handler(connection), logger(spdlog::get("jpod"))
    {
    }
    void list_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_summary(payload);
        logger->info("payload size: {}", payload.size());
        ranges::for_each(
            order.entries,
            [this](const auto &entry)
            {
                logger->info("{} {} {} {}",
                             entry.identifier,
                             entry.repository,
                             entry.tag,
                             entry.size);
            }); 
        
        send_frame(pack_summary(order));
        send_success("sent image list");
    }
    void list_handler::on_connection_closed(const std::error_code &error)
    {
        logger->info("list handler connection closed");
    }
    list_handler::~list_handler()
    {
    }
}