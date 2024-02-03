#include <domain/containers/logging_handler.h>
#include <domain/containers/repository.h>
#include <domain/containers/runtime.h>
#include <domain/containers/orders.h>
#include <spdlog/spdlog.h>

namespace domain::containers
{
    logging_handler::logging_handler(
        core::connections::connection &connection,
        std::shared_ptr<container_repository> repository,
        std::shared_ptr<runtime> runtime_ptr) : command_handler(connection),
                                                repository(repository),
                                                runtime_ptr(runtime_ptr),
                                                logger(spdlog::get("jpod"))
    {
    }

    void logging_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        // auto order = unpack_container_termed_order(payload);
    }
    void logging_handler::on_connection_closed(const std::error_code &error)
    {
    }
    logging_handler::~logging_handler()
    {
    }
}