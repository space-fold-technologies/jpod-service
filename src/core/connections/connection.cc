#include <core/connections/connection.h>
#include <core/commands/command_handler_registry.h>
#include <core/commands/command_handler.h>
#include <core/connections/frame.h>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <asio/awaitable.hpp>
#include <spdlog/spdlog.h>
#include <istream>

namespace core::connections
{
    connection::connection(
        std::string identifier,
        asio::local::stream_protocol::socket socket,
        std::shared_ptr<core::commands::command_handler_registry> command_handler_registry,
        removal_trigger_callback removal_callback) : identifier(std::move(identifier)),
                                                     socket(std::move(socket)),
                                                     buffer(10 + (1024 * 128)),
                                                     command_handler_registry(command_handler_registry),
                                                     removal_callback(removal_callback),
                                                     command_handler(nullptr),
                                                     logger(spdlog::get("jpod"))
    {
    }
    void connection::start()
    {
        read_payload();
    }
    void connection::read_payload()
    {
        socket.async_read_some(
            asio::buffer(buffer),
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        logger->info("received about {} bytes", bytes_transferred);
                        auto frame = decode_frame(buffer);
                        if (!command_handler || command_handler_key != command_handler_registry->key(frame.target, frame.operation))
                        {
                            if (auto result = command_handler_registry->fetch(frame.target, frame.operation); result.has_value())
                            {
                                auto provider = result.value();
                                if(command_handler)
                                {
                                    command_handler.reset();
                                }
                                command_handler = provider(*this);
                                command_handler_key = command_handler_registry->key(frame.target, frame.operation);
                            }
                        }
                        if (command_handler)
                        {
                            command_handler->on_order_received(frame.payload);
                        }
                    }
                    read_payload();
                }
                else
                {
                    this->on_error(err);
                }
            });
    }
    void connection::write(std::vector<uint8_t> &payload)
    {
        sending_queue.emplace_back(payload);
        if (sending_queue.size() == 1)
        {
            send_from_queue();
        }
    }
    void connection::on_error(const std::error_code &error)
    {
        if (command_handler)
        {
            command_handler->on_connection_closed(error);
        }
        removal_callback(identifier);
        if (error != asio::error::eof)
        {
            logger->error("connection error: {}", error.message());
        }
    }
    void connection::send_from_queue()
    {
        // will have to be smart about this one so as to not be penalized for using recursions
        // or maybe it is fine and your just being stupid
        auto content = sending_queue.front();
        socket.async_write_some(
            asio::buffer(content),
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        this->sending_queue.pop_front();
                        if (!this->sending_queue.empty())
                        {
                            this->send_from_queue();
                        }
                    }
                }
                else
                {
                    this->on_error(err);
                }
            });
    }
    connection::~connection()
    {
        if (socket.is_open())
        {
            socket.close();
        }
        logger->info("cleaned out");
    }
}