#include <domain/containers/shell_handler.h>
#include <domain/containers/virtual_terminal.h>
#include <domain/containers/repository.h>
#include <domain/containers/orders.h>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/split.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

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
        case shell_order_type::start_session:
        {
            auto identifier = std::string(order.data.begin(), order.data.end());
            if (auto result = repository->first_identifier_match(identifier); !result)
            {
                send_error(fmt::format("no matching container found for name: {}", identifier));
            }
            else
            {
                terminal = provider(*result, *this);
                if (auto error = terminal->initialize(); error)
                {
                    send_error(fmt::format("failed to initialize terminal: {}", error.message()));
                }
                else
                {
                    terminal->initialize();
                    logger->info("terminal for: {} initializing", identifier);
                    return;
                }
            }
        }
        case shell_order_type::terminal_size:
        {
            auto value = std::string(order.data.begin(), order.data.end());
            auto parts = value | ranges::views::split(':') | ranges::to<std::vector<std::string>>();
            auto columns = std::stoi(parts.at(0));
            auto rows = std::stoi(parts.at(1));
            terminal->resize(columns, rows);
            send_success("terminal session resized");
        }
        case shell_order_type::terminal_feed:
        {
            terminal->write(order.data);
            break;
        }
        }
    }
    void shell_handler::on_terminal_initialized()
    {
        send_success("terminal session initialized");
        terminal->start();
    }
    void shell_handler::on_terminal_data_received(const std::vector<uint8_t> &content)
    {
        send_frame(content);
    }
    void shell_handler::on_terminal_error(const std::error_code &error)
    {
        logger->info("terminal error: {}", error.message());
        send_error(fmt::format("terminal error: {}", error.message()));
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