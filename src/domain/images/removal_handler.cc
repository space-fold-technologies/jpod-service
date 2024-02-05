#include <domain/images/removal_handler.h>
#include <domain/images/repository.h>
#include <domain/images/payload.h>
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
    void image_removal_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_image_term_order(payload);
        if (repository->has_containers(order.term))
        {
            send_error(std::make_error_code(std::errc::device_or_resource_busy));
        } else if (auto error = repository->remove(order.term); error)
        {
            send_error(error);
        }
        else
        {
            send_success("image removed");
        }
    }
    void image_removal_handler::on_connection_closed(const std::error_code &error)
    {
    }
    image_removal_handler::~image_removal_handler()
    {
    }
}