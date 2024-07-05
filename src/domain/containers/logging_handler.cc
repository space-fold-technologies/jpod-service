#include <domain/containers/logging_handler.h>
#include <domain/containers/repository.h>
#include <domain/containers/runtime.h>
#include <domain/containers/orders.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace domain::containers
{
    logging_handler::logging_handler(
        core::connections::connection &connection,
        std::shared_ptr<container_repository> repository,
        std::shared_ptr<runtime> runtime_ptr) : command_handler(connection),
                                                repository(repository),
                                                runtime_ptr(runtime_ptr),
                                                container_ptr(nullptr),
                                                logger(spdlog::get("jpod"))
    {
    }

    void logging_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_log_order(payload);
        if (auto result = repository->first_identifier_match(order.name); !result.has_value())
        {
            send_error(fmt::format("failed to find a container matching: {}", order.name));
        }
        else if (container_ptr = runtime_ptr->fetch_container(result.value()); !container_ptr)
        {
            send_error(fmt::format("failed to find a container running instance for: {}", order.name));
        }
        else
        {
            container_ptr->register_listener(weak_from_this());
        }
    }
    void logging_handler::on_operation_initialization()
    {
        send_success("logging session started");
    }
    void logging_handler::on_operation_output(const std::vector<uint8_t> &content)
    {
        send_frame(content);
    }
    void logging_handler::on_operation_failure(const std::error_code &error)
    {
        send_error(fmt::format("operation failure: {}", error.message()));
    }
    listener_category logging_handler::type()
    {
        return listener_category::observer;
    }
    void logging_handler::on_connection_closed(const std::error_code &error)
    {
        logger->info("closing connection to logging handler");
    }
    logging_handler::~logging_handler()
    {
    }
}