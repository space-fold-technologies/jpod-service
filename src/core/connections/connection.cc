#include <core/connections/connection.h>
#include <core/commands/command_handler_registry.h>
#include <core/commands/command_handler.h>
#include <asio/read.hpp>
#include <asio/read_until.hpp>
#include <asio/write.hpp>
#include <asio/bind_executor.hpp>
#include <asio/awaitable.hpp>
#include <spdlog/spdlog.h>
#include <chrono>
#include <istream>

namespace core::connections
{
    connection::connection(
        std::string identifier,
        asio::local::stream_protocol::socket socket,
        std::shared_ptr<core::commands::command_handler_registry> command_handler_registry,
        removal_trigger_callback removal_callback) : identifier(std::move(identifier)),
                                                     socket(std::move(socket)),
                                                     buffer(1024),
                                                     command_handler_registry(command_handler_registry),
                                                     removal_callback(removal_callback),
                                                     command_handler(nullptr),
                                                     logger(spdlog::get("jpod"))
    {
    }
    void connection::start()
    {
        // read_payload();
        read_header();
    }
    void connection::write(uint8_t target, uint8_t operation, const std::vector<uint8_t> &payload)
    {
        std::vector<uint8_t> header;
        std::size_t length = payload.size();
        header.assign(2 + (length >= 126 ? 2 : 0) + (length >= 65536 ? 6 : 0), 0);
        header[0] = (operation & _header_operation_mask) | (target & _header_target_mask);
        if (length < 126)
        {
            header[1] = (length & 0xff) | 0;
        }
        else if (length < 65536)
        {
            header[1] = 126 | 0;
            header[2] = (length >> 8) & 0xff;
            header[3] = (length >> 0) & 0xff;
        }
        else
        {
            header[1] = 127 | 0;
            header[2] = (length >> 56) & 0xff;
            header[3] = (length >> 48) & 0xff;
            header[4] = (length >> 40) & 0xff;
            header[5] = (length >> 32) & 0xff;
            header[6] = (length >> 24) & 0xff;
            header[7] = (length >> 16) & 0xff;
            header[8] = (length >> 8) & 0xff;
            header[9] = (length >> 0) & 0xff;
        }
        std::vector<uint8_t> frame(header.begin(), header.end());
        frame.insert(frame.end(), payload.begin(), payload.end());
        sending_queue.emplace_back(std::move(frame));
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
    void connection::read_header()
    {
        this->initiate_timeout();
        asio::async_read(
            socket,
            buffer,
            asio::transfer_exactly(2),
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                this->cancel_timeout();
                if (!err)
                {
                    if (bytes_transferred == 0)
                    {
                        this->read_header();
                    }
                    else
                    {
                        std::array<uint8_t, 2> first_bytes;
                        std::istream istream(&this->buffer);
                        istream.read((char *)&first_bytes[0], 2);
                        auto target = (first_bytes[0] & _header_target_mask);
                        auto operation = (first_bytes[0] & _header_operation_mask);
                        std::size_t boundary = (first_bytes[1] & 0x7F);
                        if (boundary <= 125)
                        {
                            this->read_payload(target, operation, boundary);
                        }
                        else if (boundary == 126)
                        {
                            this->read_medium_payload_length(target, operation);
                        }
                        else if (boundary == 127)
                        {
                            this->read_large_payload_length(target, operation);
                        }
                    }
                }
                else
                {
                    this->on_error(err);
                }
            });
    }
    void connection::read_payload(uint8_t target, uint8_t operation, std::size_t payload_length)
    {
        initiate_timeout();
        asio::async_read(
            socket,
            buffer,
            asio::transfer_exactly(payload_length),
            [this, target, operation, payload_length](const std::error_code &err, std::size_t bytes_transferred)
            {
                this->cancel_timeout();
                if (!err)
                {
                    std::istream input_stream(&this->buffer);

                    std::vector<uint8_t> content;
                    content.reserve(payload_length);
                    for (std::size_t c = 0; c < static_cast<std::size_t>(payload_length); c++)
                    {
                        content.push_back(input_stream.get());
                    }
                    this->handle_payload(target, operation, content);

                    this->read_header();
                }
                else
                {
                    this->on_error(err);
                }
            });
    }
    void connection::read_medium_payload_length(uint8_t target, uint8_t operation)
    {
        initiate_timeout();
        asio::async_read(
            socket,
            buffer,
            asio::transfer_exactly(2),
            [this, target, operation](const std::error_code &err, std::size_t bytes_transferred)
            {
                this->cancel_timeout();
                if (!err)
                {
                    std::istream istream(&this->buffer);
                    std::size_t length = 0;
                    std::size_t num_bytes = 2;
                    std::array<uint8_t, 2> length_bytes;
                    istream.read((char *)&length_bytes[0], 2);
                    for (std::size_t c = 0; c < num_bytes; c++)
                    {
                        length += static_cast<std::size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));
                    }
                    this->read_payload(target, operation, length);
                }
                else
                {
                    this->on_error(err);
                }
            });
    }
    void connection::read_large_payload_length(uint8_t target, uint8_t operation)
    {
        initiate_timeout();
        asio::async_read(
            socket,
            buffer,
            asio::transfer_exactly(8),
            [this, target, operation](const std::error_code &err, std::size_t bytes_transferred)
            {
                this->cancel_timeout();
                if (!err)
                {
                    std::istream istream(&this->buffer);
                    std::size_t length = 0;
                    std::size_t num_bytes = 8;
                    std::array<uint8_t, 8> length_bytes;
                    istream.read((char *)&length_bytes[0], 8);
                    for (std::size_t c = 0; c < num_bytes; c++)
                    {
                        length += static_cast<std::size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));
                    }
                    this->read_payload(target, operation, length);
                }
                else
                {
                    this->on_error(err);
                }
            });
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
                        sending_queue.pop_front();
                        if (!sending_queue.empty())
                        {
                            send_from_queue();
                        }
                    }
                }
                else
                {
                    this->on_error(err);
                }
            });
    }
    void connection::cancel_timeout()
    {
    }
    void connection::initiate_timeout()
    {
    }
    void connection::handle_payload(uint8_t target, uint8_t operation, const std::vector<uint8_t> &payload)
    {
        auto _target = static_cast<operation_target>(target);
        auto _operation = static_cast<request_operation>(operation);
        if (!command_handler || command_handler_key != command_handler_registry->key(_target, _operation))
        {
            if (auto result = command_handler_registry->fetch(_target, _operation); result.has_value())
            {
                auto provider = result.value();
                if (command_handler)
                {
                    command_handler.reset();
                }
                command_handler = provider(*this);
                command_handler_key = command_handler_registry->key(_target, _operation);
            }
        }
        if (command_handler)
        {
            command_handler->on_order_received(payload);
        }
    }
    connection::~connection()
    {
        if (socket.is_open())
        {
            socket.close();
        }
    }
}