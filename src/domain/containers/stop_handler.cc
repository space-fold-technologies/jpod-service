#include <domain/containers/stop_handler.h>
#include <domain/containers/repository.h>
#include <domain/containers/container.h>
#include <domain/containers/runtime.h>
#include <domain/containers/orders.h>
#include <asio/io_context.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

namespace domain::containers
{
    stop_handler::stop_handler(
        core::connections::connection &connection,
        std::shared_ptr<container_repository> repository,
        std::shared_ptr<runtime> runtime_ptr,
        const fs::path &containers_folder) : command_handler(connection),
                                             repository(repository),
                                             runtime_ptr(runtime_ptr),
                                             containers_folder(containers_folder),
                                             logger(spdlog::get("jpod"))
    {
    }

    void stop_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_term_order(payload);
        // create the operation details from querying the database
        if (auto result = repository->first_match(order.term); !result.has_value())
        {
            // have to come up with custom errors for containers
            send_error(std::make_error_code(std::errc::no_such_process));
        }
        else
        {
            runtime_ptr->remove_container(result->identifier);
            send_success(fmt::format("container with id: {} stopped", result->identifier));
        }
    }
    void stop_handler::on_connection_closed(const std::error_code &error)
    {
        logger->debug("stopping stop handler");
    }
    stop_handler::~stop_handler()
    {
    }
}