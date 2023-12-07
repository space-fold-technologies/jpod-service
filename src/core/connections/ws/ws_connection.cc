#include <core/connections/ws/ws_connection.h>
#include <core/connections/ws/crypto.h>
#include <core/connections/ws/headers.h>
#include <core/connections/ws/types.h>
#include <core/connections/ws/ws_properties_listener.h>
#include <core/connections/connection_listener.h>
#include <thread>
#include <algorithm>
#include <asio/write.hpp>
#include <asio/read.hpp>
#include <asio/read_until.hpp>
#include <asio/bind_executor.hpp>
#include <asio/connect.hpp>
#include <chrono>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <random>

namespace core::connections::ws
{
    WebsocketConnection::WebsocketConnection(asio::ip::tcp::socket socket, PropertiesListener &properties_callback) : socket(std::move(socket)),
                                                                                                                      properties_callback(properties_callback),
                                                                                                                      listener(nullptr),
                                                                                                                      message_ptr(std::make_unique<message>()),
                                                                                                                      logger(spdlog::get("jpod")),
                                                                                                                      timer(std::unique_ptr<asio::steady_timer>(
                                                                                                                          new asio::steady_timer(
                                                                                                                              socket.get_executor(),
                                                                                                                              std::chrono::seconds(10))))
    {
        message_ptr->data = std::vector<uint8_t>{};
        message_ptr->opcode = 0x00;
        message_ptr->segments = 0;
    }
    void WebsocketConnection::initiate()
    {
        read_request_handshake();
    }
    void WebsocketConnection::connect()
    {
        socket.async_wait(
            asio::socket_base::wait_read,
            [this](const std::error_code &err)
            {
            if(!err)
            {
                this->listener->on_open();
                std::thread t(&WebsocketConnection::read_payload_header, this);
                t.join();
            } else 
            {
                this->listener->on_error(err);
            } });
    }
    void WebsocketConnection::register_callback(std::shared_ptr<ConnectionListener> listener)
    {
        this->listener = listener;
    }
    void WebsocketConnection::write(const std::vector<uint8_t> &payload)
    {
        send_payload(OP_BINARY, true, true, payload);
    }
    void WebsocketConnection::read_request_handshake()
    {

        this->initiate_timeout();
        asio::async_read_until(
            socket,
            stream_buffer,
            "\r\n\r\n",
            [this](const std::error_code &err, size_t bytes_transferred)
            {
                this->cancel_timeout();
                if (!err && bytes_transferred > 0)
                {
                    std::istream stream(&this->stream_buffer);
                    auto result = Request::parse(stream);
                    if (result.has_value())
                    {
                        this->response_to_handshake(*result);
                    }
                    else
                    {
                        this->listener->on_error(std::make_error_code(std::errc::bad_message));
                    }
                }
                else
                {
                    this->listener->on_error(err);
                }
            });
    }
    void WebsocketConnection::response_to_handshake(const Request &request)
    {
        if (auto position = request.headers.find("Sec-WebSocket-Key"); position != request.headers.end())
        {
            static auto ws_magic_string = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
            auto sha1 = Crypto::sha1(position->second + ws_magic_string);
            auto payload = fmt::format(
                "HTTP/1.1 101 Web Socket Protocol Handshake\r\n"
                "Upgrade: websocket\r\n"
                "Connection: Upgrade\r\n"
                "Sec-WebSocket-Accept: {}\r\n\r\n",
                Crypto::base64_encode(sha1));
            std::vector<uint8_t> content(payload.begin(), payload.end());

            asio::async_write(
                socket,
                asio::buffer(content),
                [this, path = request.path](const std::error_code &err, std::size_t bytes_transferred)
                {
                    // this->cancel_timeout();
                    if (!err && bytes_transferred > 0)
                    {
                        this->properties_callback.on_request(this->shared_from_this(), path);
                    }
                    else
                    {
                        this->listener->on_error(err);
                        this->socket.close();
                    }
                });
            // this->initiate_timeout();
        }
        else
        {
            auto status = StatusCode::error_upgrade_required;
            auto error_message = status_code_value(status);
            auto details = std::string("You need to upgrade your client");
            status_report(static_cast<int>(status), error_message, details);
        }
    }

    void WebsocketConnection::read_payload_header()
    {
        this->initiate_timeout();
        asio::async_read(
            socket,
            stream_buffer,
            asio::transfer_exactly(2),
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                this->cancel_timeout();
                if (!err)
                {
                    if (bytes_transferred == 0)
                    {
                        this->read_payload_header();
                    }
                    else
                    {
                        std::array<uint8_t, 2> first_bytes;
                        std::istream istream(&this->stream_buffer);
                        istream.read((char *)&first_bytes[0], 2);
                        bool fin = first_bytes[0] >> 7;
                        bool has_mask = first_bytes[1] >> 7;
                        uint8_t opcode = first_bytes[0] & 0x0F;
                        std::size_t boundary = (first_bytes[1] & 0x7F);
                        if (boundary <= 125)
                        {
                            this->read_payload_content(fin, opcode, has_mask, boundary);
                        }
                        else if (boundary == 126)
                        {
                            this->read_medium_payload(fin, opcode, has_mask);
                        }
                        else if (boundary == 127)
                        {
                            this->read_large_payload(fin, opcode, has_mask);
                        }
                    }
                }
                else
                {
                    this->listener->on_error(err);
                }
            });
    }

    void WebsocketConnection::read_medium_payload(bool fin, uint8_t opcode, bool has_mask)
    {
        initiate_timeout();
        asio::async_read(
            socket,
            stream_buffer,
            asio::transfer_exactly(2),
            [this, fin, opcode, has_mask](const std::error_code &err, std::size_t bytes_transferred)
            {
                this->cancel_timeout();
                if (!err)
                {
                    std::istream istream(&this->stream_buffer);
                    std::size_t length = 0;
                    std::size_t num_bytes = 2;
                    std::array<uint8_t, 2> length_bytes;
                    istream.read((char *)&length_bytes[0], 2);
                    for (std::size_t c = 0; c < num_bytes; c++)
                    {
                        length += static_cast<std::size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));
                    }
                    this->read_payload_content(fin, opcode, has_mask, length);
                }
                else
                {
                    this->listener->on_error(err);
                }
            });
    }

    void WebsocketConnection::read_large_payload(bool fin, uint8_t opcode, bool has_mask)
    {
        initiate_timeout();
        asio::async_read(
            socket,
            stream_buffer,
            asio::transfer_exactly(8),
            [this, fin, opcode, has_mask](const std::error_code &err, std::size_t bytes_transferred)
            {
                this->cancel_timeout();
                if (!err)
                {
                    std::istream istream(&this->stream_buffer);
                    std::size_t length = 0;
                    std::size_t num_bytes = 8;
                    std::array<uint8_t, 8> length_bytes;
                    istream.read((char *)&length_bytes[0], 8);
                    for (std::size_t c = 0; c < num_bytes; c++)
                    {
                        length += static_cast<std::size_t>(length_bytes[c]) << (8 * (num_bytes - 1 - c));
                    }
                    this->read_payload_content(fin, opcode, has_mask, length);
                }
                else
                {
                    this->listener->on_error(err);
                }
            });
    }

    void WebsocketConnection::read_payload_content(bool fin, uint8_t opcode, bool has_mask, std::size_t payload_length)
    {
        std::size_t bytes_to_fetch = payload_length + (has_mask ? 4 : 0);
        initiate_timeout();
        asio::async_read(
            socket,
            stream_buffer,
            asio::transfer_exactly(bytes_to_fetch),
            [this, fin, opcode, has_mask, payload_length](const std::error_code &err, std::size_t bytes_transferred)
            {
                this->cancel_timeout();
                if (!err)
                {
                    std::istream input_stream(&this->stream_buffer);

                    std::vector<uint8_t> content;
                    content.reserve(payload_length);
                    std::array<uint8_t, 4> mask_key;
                    if (has_mask)
                    {
                        std::array<unsigned char, 4> mask;
                        input_stream.read((char *)&mask[0], 4);
                        for (std::size_t c = 0; c < static_cast<std::size_t>(payload_length); c++)
                        {
                            content.push_back(input_stream.get() ^ mask[c % 4]);
                        }
                    }
                    else
                    {
                        for (std::size_t c = 0; c < static_cast<std::size_t>(payload_length); c++)
                        {
                            content.push_back(input_stream.get());
                        }
                    }
                    if (opcode == OP_PING)
                    {
                        this->send_payload(OP_PONG, true, mask_key, content);
                        // this->listener->on_ping(content);
                    }
                    else if (opcode == OP_PONG)
                    {
                        // this->listener->on_pong(content);
                    }
                    else if (opcode == OP_CLOSE)
                    {
                        // this->send(OP_CLOSE); <<-- find out how this is meant to be done
                        this->listener->on_close();
                    }
                    else
                    {
                        this->handle_fragment(fin, opcode, content);
                    }
                    this->read_payload_header();
                }
                else
                {
                    this->listener->on_error(err);
                }
            });
    }

    void WebsocketConnection::handle_fragment(bool fin, uint8_t opcode, const std::vector<uint8_t> &content)
    {
        if (opcode == OP_BINARY || opcode == OP_TEXT)
        {
            message_ptr->data.clear();
            message_ptr->opcode = opcode;
            message_ptr->segments = 0;
        }
        message_ptr->data.insert(this->message_ptr->data.end(), content.begin(), content.end());
        message_ptr->segments++;
        if (fin)
        {
            this->listener->on_message(message_ptr->data);
        }
    }
    void WebsocketConnection::encode_length_to_frame(std::vector<uint8_t> &header, bool send_mask, const std::array<uint8_t, 4> &mask, std::size_t length)
    {
        if (length < 126)
        {
            header[1] = (length & 0xff) | (send_mask ? 0x80 : 0);
            if (send_mask)
            {
                header[2] = mask[0];
                header[3] = mask[1];
                header[4] = mask[2];
                header[5] = mask[3];
            }
        }
        else if (length < 65536)
        {
            header[1] = 126 | (send_mask ? 0x80 : 0);
            header[2] = (length >> 8) & 0xff;
            header[3] = (length >> 0) & 0xff;
            if (send_mask)
            {
                header[4] = mask[0];
                header[5] = mask[1];
                header[6] = mask[2];
                header[7] = mask[3];
            }
        }
        else
        {
            header[1] = 127 | (send_mask ? 0x80 : 0);
            header[2] = (length >> 56) & 0xff;
            header[3] = (length >> 48) & 0xff;
            header[4] = (length >> 40) & 0xff;
            header[5] = (length >> 32) & 0xff;
            header[6] = (length >> 24) & 0xff;
            header[7] = (length >> 16) & 0xff;
            header[8] = (length >> 8) & 0xff;
            header[9] = (length >> 0) & 0xff;
            if (send_mask)
            {
                header[10] = mask[0];
                header[11] = mask[1];
                header[12] = mask[2];
                header[13] = mask[3];
            }
        }
    }

    void WebsocketConnection::send_payload(uint8_t opcode, bool fin, bool send_mask, const std::vector<uint8_t> &content)
    {
        std::size_t length = content.size();
        std::vector<uint8_t> header;
        header.assign(2 + (length >= 126 ? 2 : 0) + (length >= 65536 ? 6 : 0) + (send_mask ? 4 : 0), 0);
        header[0] = ((opcode & 15) | ((uint8_t)fin << 7));
        std::array<uint8_t, 4> mask;
        if (send_mask)
        {
            std::uniform_int_distribution<unsigned short> dist(0, 255);
            std::random_device rd;
            for (std::size_t c = 0; c < 4; c++)
            {
                mask[c] = static_cast<uint8_t>(dist(rd));
            }
        }
        encode_length_to_frame(header, send_mask, mask, length);
        std::vector<uint8_t> frame(header.begin(), header.end());
        if (send_mask)
        {
            for (std::size_t c = 0; c < length; c++)
            {
                frame.push_back(content.at(c) ^ mask[c & 0x3]);
            }
        }
        else
        {
            frame.insert(frame.end(), content.begin(), content.end());
        }
        sending_queue.emplace_back(std::move(frame));
        if (sending_queue.size() == 1)
        {
            send_from_queue();
        }
    }
    void WebsocketConnection::send_payload(uint8_t opcode, bool fin, const std::array<uint8_t, 4> &mask, const std::vector<uint8_t> &content)
    {
        bool send_mask = true;
        std::size_t length = content.size();
        std::vector<uint8_t> header(2 + (length >= 126 ? 2 : 0) + (length >= 65536 ? 6 : 0) + (send_mask ? 4 : 0), 0);
        header[0] = ((opcode & 15) | ((uint8_t)fin << 7));
        encode_length_to_frame(header, send_mask, mask, length);
        std::vector<uint8_t> frame(header.begin(), header.end());
        for (std::size_t c = 0; c < length; c++)
        {
            frame.push_back(content.at(c) ^ mask[c & 0x3]);
        }
        sending_queue.emplace_back(std::move(frame));
        if (sending_queue.size() == 1)
        {
            send_from_queue();
        }
    }

    void WebsocketConnection::send_from_queue()
    {
        // will have to be smart about this one so as to not be penalized for using recursions
        // or maybe it is fine and your just being stupid
        auto content = sending_queue.front();
        asio::async_write(
            socket,
            asio::buffer(content),
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
          if(!err)
          {
            this->sending_queue.pop_front();
            if(!this->sending_queue.empty())
            {
              this->send_from_queue();
            }
          } else {
            this->listener->on_error(err);
          } });
    }

    void WebsocketConnection::initiate_timeout()
    {

        // if (lock.try_lock())
        // {
        //     std::weak_ptr<Connection> self(shared_from_this());
        //     timer->async_wait(
        //         [self](const std::error_code &err)
        //         {
        //         if(!err)
        //         {
        //             if(auto connection = self.lock())
        //             {
        //             // send a message to represent a timeout or something civil
        //             connection->socket.close();
        //             }
        //         } else {
        //             spdlog::get("jpod")->error("Failure to acquire timed lock: {}", err.message());
        //         } });
        // }
    }
    void WebsocketConnection::cancel_timeout()
    {
        // lock.unlock();
        // timer->cancel();
    }
    void WebsocketConnection::status_report(int status, std::string &reason, std::string &details)
    {
        std::string headers = fmt::format(
            "HTTP/1.1 {0} {1}\r\n"
            "Content-Length: {2}\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n\r\n"
            "{3}",
            status,
            reason,
            details);
        this->initiate_timeout();
        asio::async_write(
            socket,
            asio::buffer(headers),
            [this](const std::error_code &error, size_t bytes_transferred)
            {
                this->cancel_timeout();
                if (bytes_transferred > 0)
                {
                    this->socket.close();
                    this->listener->on_close();
                }
            });
    }

    void WebsocketConnection::disconnect()
    {
        // TODO:: close connection properly
    }
}