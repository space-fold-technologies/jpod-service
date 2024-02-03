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
                                                container_ptr(nullptr),
                                                logger(spdlog::get("jpod"))
    {
    }

    void logging_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_term_order(payload);
        if (auto result = repository->first_identifier_match(order.term); !result)
        {
            // TODO: have to come up with custom errors for containers
            send_error(std::make_error_code(std::errc::no_such_process));
        }
        else if (container_ptr = runtime_ptr->fetch_container(*result); !container_ptr)
        {
            // TODO: have to come up with custom errors for containers
            send_error(std::make_error_code(std::errc::no_such_process));
        }
        else
        {
            container_ptr->register_listener(shared_from_this());
        }
    }
    void logging_handler::on_operation_initialization()
    {
    }
    void logging_handler::on_operation_output(const std::vector<uint8_t> &content)
    {
    }
    void logging_handler::on_operation_failure(const std::error_code &error)
    {
    }
    listener_category logging_handler::type()
    {
        return listener_category::observer;
    }
    void logging_handler::on_connection_closed(const std::error_code &error)
    {
    }
    logging_handler::~logging_handler()
    {
    }
}