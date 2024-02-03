#include <domain/containers/shell_handler.h>
#include <domain/containers/virtual_terminal.h>
#include <domain/containers/repository.h>
#include <domain/containers/orders.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split.hpp>
#include <spdlog/spdlog.h>

namespace domain::containers
{
    shell_handler::shell_handler(
        core::connections::connection &connection,
        std::shared_ptr<container_repository> repository,
        virtual_terminal_provider provider) : command_handler(connection),
                                              provider(std::move(provider)),
                                              repository(repository),
                                              logger(spdlog::get("jpod"))
    {
    }

    void shell_handler::on_order_received(const std::vector<uint8_t> &payload)
    {
        auto order = unpack_container_shell_order(payload);
        switch (order.type)
        {
        case shell::start_session:
        {
            if (auto result = repository->first_identifier_match(std::string(order.data.begin(), order.data.end())); !result)
            {
                // TODO: have to come up with custom errors for containers
                send_error(std::make_error_code(std::errc::no_such_process));
            }
            else
            {
                terminal = provider(*result, *this);
                terminal->start();
            }
        }
        case shell::terminal_size:
        {
            auto value = std::string(order.data.begin(), order.data.end());
            auto parts = value | ranges::view::split(':') | ranges::to<std::vector<std::string>>();
            auto columns = std::stoi(parts.at(0));
            auto rows = std::stoi(parts.at(1));
            terminal->resize(columns, rows);
            send_success("terminal session resized");
        }
        case shell::terminal_feed:
        {
            terminal->write(order.data);
            break;
        }
        }
    }
    void shell_handler::on_terminal_initialized()
    {
        send_success("terminal session created");
    }
    void shell_handler::on_terminal_data_received(const std::vector<uint8_t> &content)
    {
        send_frame(content);
    }
    void shell_handler::on_terminal_error(const std::error_code &error)
    {
        send_error(error);
    }
    void shell_handler::on_connection_closed(const std::error_code &error)
    {
        if (error)
        {
            logger->error("client connection: {}", error.message());
        }
    }
    shell_handler::~shell_handler()
    {
        if (terminal)
        {
            terminal.reset();
        }
    }
}