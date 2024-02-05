#include <domain/containers/removal_handler.h>
#include <domain/containers/repository.h>

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
    }
    void container_removal_handler::on_connection_closed(const std::error_code &error)
    {
    }
    container_removal_handler::~container_removal_handler()
    {
    }
}