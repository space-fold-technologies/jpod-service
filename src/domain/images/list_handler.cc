#include <domain/images/list_handler.h>
#include <domain/images/repository.h>
#include <domain/images/payload.h>
#include <spdlog/spdlog.h>

namespace domain::images
{
    image_list_handler::image_list_handler(core::connections::connection &connection, std::shared_ptr<image_repository> repository) : command_handler(connection),
                                                                                                                                      repository(repository),
                                                                                                                                      logger(spdlog::get("jpod"))
    {
    }
    void image_list_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_image_term_order(payload);
        auto entries = repository->fetch_matching_details(order.term);
        send_frame(pack_image_entries(entries));
    }
    void image_list_handler::on_connection_closed(const std::error_code &error)
    {
        logger->info("image list handler connection closed");
    }
    image_list_handler::~image_list_handler()
    {
    }
}