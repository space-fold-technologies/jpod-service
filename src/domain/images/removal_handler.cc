#include <domain/images/removal_handler.h>
#include <domain/images/repository.h>
#include <spdlog/spdlog.h>

namespace domain::images
{
    image_removal_handler::image_removal_handler(
        core::connections::connection &connection,
        std::shared_ptr<image_repository> repository) : command_handler(connection),
                                                        repository(repository),
                                                        logger(spdlog::get("jpod"))
    {
    }
    image_removal_handler::~image_removal_handler()
    {
    }
    void image_removal_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
    }
    void image_removal_handler::on_connection_closed(const std::error_code &error)
    {
    }
}