#include <domain/images/http/connection_base.h>
#include <domain/images/http/lifetime_listener.h>
#include <domain/images/http/contracts.h>
#include <domain/images/http/download_destination.h>
#include <domain/images/http/request.h>
#include <asio/write.hpp>
#include <asio/read.hpp>
#include <asio/read_until.hpp>
#include <asio/io_context.hpp>
#include <asio/bind_executor.hpp>
#include <arpa/inet.h>
#include <spdlog/spdlog.h>

namespace domain::images::http
{
    connection_base::connection_base(asio::io_context &context, const std::string &identifier, lifetime_listener &listener) : session(nullptr),
                                                                                                                              download_ptr(nullptr),
                                                                                                                              logger(spdlog::get("jpod")),
                                                                                                                              listener(listener),
                                                                                                                              identifier(identifier),
                                                                                                                              resolver(context),
                                                                                                                              file_stream(context),
                                                                                                                              file_chunk_buffer(FILE_CHUNK_SIZE)
    {
    }
    asio::ip::tcp::resolver::results_type connection_base::resolve_endpoints(const std::string &host, uint16_t port)
    {
        if (is_ip_address(host))
        {
            asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(host), port);
            return resolver.resolve(endpoint);
        }

        asio::ip::tcp::resolver::query query(host, fmt::format("{}", port), asio::ip::tcp::resolver::query::numeric_service);
        return resolver.resolve(query);
    }

    bool connection_base::is_ip_address(const std::string &ip)
    {
        struct sockaddr_in socket_address;
        if ((inet_pton(AF_INET, ip.c_str(), &(socket_address.sin_addr))))
        {
            return true;
        }
        return false;
    }

    void connection_base::write_call(request req)
    {
        asio::streambuf out;
        req.write(out);
        asio::async_write(
            this->socket(),
            out,
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err && bytes_transferred > 0)
                {
                    this->logger->info("HTTP DATA WRITTEN");
                    this->read_response_header();
                }
                else
                {
                    this->on_request_failure(err);
                }
            });
    }

    void connection_base::fetch_file_details(request req)
    {
        asio::streambuf out;
        req.write(out);
        asio::async_write(
            this->socket(),
            out,
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err && bytes_transferred > 0)
                {
                    this->logger->info("HTTP DATA WRITTEN");
                    this->read_file_details();
                }
                else
                {
                    this->on_download_failure(err);
                }
            });
    }

    void connection_base::register_file_details(request req)
    {
        asio::streambuf out;
        req.write(out);
        asio::async_write(
            this->socket(),
            out,
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err && bytes_transferred > 0)
                {
                    this->logger->info("HTTP UPLOAD REGISTRATION DATA WRITTEN");
                    this->read_file_chunk();
                }
                else
                {
                    this->on_upload_failure(err);
                }
            });
    }

    void connection_base::read_file_chunk()
    {
        asio::async_read(
            file_stream,
            asio::buffer(file_chunk_buffer),
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (err)
                {
                    this->on_upload_failure(err);
                }
                else if (bytes_transferred > 0)
                {
                    auto status = this->upload_ptr->status;
                    std::size_t start = status.end + 1;
                    std::size_t end = status.end + bytes_transferred;
                    std::map<std::string, std::string> headers(this->upload_ptr->headers);
                    headers.try_emplace("Content-Range", fmt::format("{}={}-{}", download_ptr->status.unit, this->upload_ptr->status.start, this->upload_ptr->status.end));
                    headers.try_emplace("Content-Length", fmt::format("{}", download_ptr->status.total));
                    request req("POST", this->upload_ptr->uri, this->file_chunk_buffer, "application/octet-stream", this->upload_ptr->headers);
                    this->send_file_chunk(req, bytes_transferred);
                }
                else
                {
                    auto status = this->upload_ptr->status;
                    status.complete = true;
                    this->upload_ptr->callback({}, status);
                    this->file_stream.close();
                    this->listener.available(this->identifier);
                }
            });
    }
    void connection_base::send_file_chunk(request &req, std::size_t bytes_to_transfer)
    {
        asio::streambuf out;
        req.write(out);
        asio::async_write(
            this->socket(),
            out,
            [this, bytes_to_transfer](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err && bytes_transferred > 0)
                {   auto previous_status = this->upload_ptr->status;
                    this->upload_ptr->status.current += bytes_to_transfer;
                    this->upload_ptr->status.start = previous_status.end + 1;
                    this->upload_ptr->status.end = previous_status.end + bytes_to_transfer;
                    this->upload_ptr->callback({}, this->upload_ptr->status);
                    this->read_file_chunk();
                }
                else
                {
                    this->on_upload_failure(err);
                }
            });
    }

    void connection_base::write_fetch_chunk_request(std::size_t start, std::size_t end, std::size_t chunk_size)
    {

        std::map<std::string, std::string> headers{download_ptr->headers.begin(), download_ptr->headers.end()};
        headers.try_emplace("Range", fmt::format("{}={}-{}", download_ptr->status.unit, start, end));
        request request("GET", download_ptr->uri, std::vector<uint8_t>{}, "", headers);
        auto endpoints = resolve_endpoints(request.host(), request.port());
        asio::streambuf out;
        request.write(out);
        asio::async_write(
            this->socket(),
            out,
            [this, start, end, chunk_size](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err && bytes_transferred > 0)
                {
                    this->fetch_chunk_header(start, end, chunk_size);
                }
                else
                {
                    this->on_download_failure(err);
                }
            });
    }

    void connection_base::fetch_chunk_header(std::size_t start, std::size_t end, std::size_t chunk_size)
    {
        asio::async_read_until(
            this->socket(),
            this->buffer,
            "\r\n\r\n",
            [this, start, end, chunk_size](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        std::stringstream ss;
                        ss << &this->buffer;
                        std::string headers_contained = ss.str();
                        if (auto headers_end = headers_contained.find("\r\n\r\n"); headers_end != std::string::npos)
                        {
                            std::string exact_header_content = headers_contained.substr(0, headers_end + 4);
                            this->buffer.consume(exact_header_content.size());
                            std::error_code error;
                            if (auto response = parse_response(exact_header_content, error); error)
                            {
                                this->on_download_failure(error);
                            }
                            else if (auto pos = response.headers.find("Content-Range"); pos != response.headers.end())
                            {
                                auto range_details = pos->second;
                                auto range_captured = range_details.replace(range_details.find(this->download_ptr->status.unit), this->download_ptr->status.unit.size(), "");

                                auto range = range_captured.substr(0, range_captured.find("/"));
                                this->download_ptr->status.start = std::stoull(range.substr(0, range.find("-")));
                                this->download_ptr->status.end = std::stoull(range.substr(range.find("-") + 1));
                                auto total_length = std::stoull(range_captured.substr(range_captured.find("/") + 1));
                                auto content_length = std::stoull(response.headers.find("Content-Length")->second);
                                this->logger->info("DELIVERED CONTENT LENGTH: {}", content_length);
                                this->logger->info("Range: {} {}-{}/{} :::: Transfer {}", this->download_ptr->status.unit, start, end, total_length, content_length);
                                this->download_ptr->status.current += content_length;
                                this->download_ptr->chunk_size = content_length;
                                std::string excess = headers_contained.substr(headers_end + 4);
                                std::size_t remaining_bytes = excess.size();
                                auto content = std::vector<uint8_t>(excess.begin(), excess.end());
                                this->download_ptr->destination->write(content);
                                this->buffer.consume(bytes_transferred);
                                size_t content_to_fetch = remaining_bytes < content_length ? content_length - remaining_bytes : content_length;
                                this->fetch_chunk_content(content_to_fetch);
                            }
                        }
                    }
                    else
                    {
                        this->fetch_chunk_header(start, end, chunk_size);
                    }
                }
                else
                {
                    this->on_download_failure(err);
                }
            });
    }

    void connection_base::fetch_chunk_content(std::size_t remaining_bytes)
    {
        logger->info("reading additional content-chunk: {}", remaining_bytes);
        asio::async_read(
            this->socket(),
            this->buffer,
            asio::transfer_exactly(remaining_bytes),
            [this, remaining_bytes](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        this->logger->info("transferred :{} bytes", bytes_transferred);
                        std::istream stream(&this->buffer);
                        std::vector<uint8_t> remainder(bytes_transferred);
                        stream.read((char *)&remainder[0], bytes_transferred);
                        this->download_ptr->destination->write(remainder);
                        this->logger->info("written to file");
                        if (remaining_bytes > bytes_transferred)
                        {
                            this->logger->info("fetching next round");
                            this->fetch_chunk_content(remaining_bytes - bytes_transferred);
                        }
                        else
                        {
                            if (this->download_ptr->status.current < this->download_ptr->status.total)
                            {
                                auto end_index = this->download_ptr->status.total - 1;
                                auto next_start = this->download_ptr->status.end + 1;
                                auto next_chunk_size = this->download_ptr->chunk_size + next_start < end_index ? this->download_ptr->chunk_size : (end_index - next_start) + 1;
                                auto next_end = (next_start + next_chunk_size) - 1;
                                this->download_ptr->status.complete = false;
                                this->download_ptr->callback({}, this->download_ptr->status);
                                this->write_fetch_chunk_request(next_start, next_end, next_chunk_size);
                            }
                            else
                            {
                                // We are done
                                this->download_ptr->status.complete = true;
                                this->download_ptr->callback({}, this->download_ptr->status);
                                this->listener.available(this->identifier);
                            }
                        }
                    }
                }
                else
                {

                    this->logger->error("borked transfer ::{}", err.message());
                    this->on_download_failure(err);
                }
            });
    }

    void connection_base::read_response_header()
    {
        asio::async_read_until(
            this->socket(),
            this->buffer,
            "\r\n\r\n",
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        std::stringstream ss;
                        ss << &this->buffer;
                        std::string headers_contained = ss.str();
                        if (auto headers_end = headers_contained.find("\r\n\r\n"); headers_end != std::string::npos)
                        {
                            std::string exact_header_content = headers_contained.substr(0, headers_end + 4);
                            this->buffer.consume(exact_header_content.size());
                            std::size_t remaining_bytes = headers_contained.substr(headers_end + 4).size();
                            std::error_code error;
                            if (auto response = parse_response(exact_header_content, error); error)
                            {
                                this->on_download_failure(error);
                            }
                            else
                            {
                                this->session->initial_response = std::move(response);
                                if (this->session->initial_response.has_body())
                                {
                                    this->logger->info("content has a body");
                                    std::size_t content_length = this->session->initial_response.content_length();
                                    this->buffer.consume(exact_header_content.size());
                                    this->read_body(remaining_bytes < content_length ? content_length - remaining_bytes : content_length);
                                }
                                else if (this->session->initial_response.is_closed())
                                {
                                    this->logger->info("is a close connection response");
                                    this->buffer.consume(bytes_transferred);
                                    this->read_body();
                                }
                                else if (this->session->initial_response.is_chunked())
                                {
                                    this->logger->info("is a chunked response");
                                    auto chunk_buffer = std::make_shared<asio::streambuf>(std::max<std::size_t>(16 + 2, this->buffer.size()));
                                    chunk_buffer->commit(asio::buffer_copy(chunk_buffer->prepare(this->buffer.size()), this->buffer.data()));
                                    this->buffer.consume(this->buffer.size());
                                    this->read_chunk_size(chunk_buffer);
                                }
                                else if (this->session->initial_response.is_event_stream())
                                {
                                    this->logger->info("is event stream");
                                    auto events_stream_buffer = std::make_shared<asio::streambuf>(std::numeric_limits<std::size_t>::max());
                                    events_stream_buffer->commit(asio::buffer_copy(events_stream_buffer->prepare(this->buffer.size()), this->buffer.data()));
                                    this->buffer.consume(this->buffer.size());
                                    this->read_event_stream(events_stream_buffer);
                                }
                            }
                        }
                    }
                    else
                    {
                        this->read_response_header();
                    }
                }
                else
                {
                    this->on_download_failure(err);
                }
            });
    }

    void connection_base::read_file_details()
    {
        asio::async_read_until(
            this->socket(),
            this->buffer,
            "\r\n\r\n",
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        std::stringstream ss;
                        ss << &this->buffer;
                        std::string headers_contained = ss.str();
                        if (auto headers_end = headers_contained.find("\r\n\r\n"); headers_end != std::string::npos)
                        {
                            std::string exact_header_content = headers_contained.substr(0, headers_end + 4);
                            this->buffer.consume(exact_header_content.size());
                            std::size_t remaining_bytes = headers_contained.substr(headers_end + 4).size();
                            std::error_code error;
                            if (auto response = parse_response(exact_header_content, error); error)
                            {
                                this->on_download_failure(error);
                            }
                            else if (response.accepts_ranges())
                            {
                                download_status status{};
                                status.total = response.content_length();
                                status.unit = response.unit();
                                status.current = 0L;
                                this->logger->info("Content-Length: {}", status.total);
                                this->logger->info("UNIT: {}", status.unit);
                                this->download_ptr->status = std::move(status);
                                this->download_ptr->callback({}, this->download_ptr->status);
                                std::size_t chunk_size = std::min<std::size_t>(
                                    this->download_ptr->destination->chunk_size(),
                                    this->download_ptr->status.total);
                                this->buffer.consume(exact_header_content.size());
                                this->write_fetch_chunk_request(this->download_ptr->status.current, chunk_size - 1, chunk_size);
                            }
                            // start the download_ptr thingy here
                        }
                    }
                    else
                    {
                        this->read_file_details();
                    }
                }
                else
                {
                    this->on_download_failure(err);
                }
            });
    }

    void connection_base::read_body(std::size_t bytes_left_over)
    {
        asio::async_read(
            this->socket(),
            this->buffer,
            asio::transfer_exactly(bytes_left_over),
            [this, bytes_left_over](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        std::istream stream(&this->buffer);
                        std::vector<uint8_t> content(bytes_transferred);
                        stream.read((char *)&content[0], bytes_transferred);
                        this->session->initial_response.data.insert(this->session->initial_response.data.end(), content.begin(), content.end());
                        this->read_body(bytes_left_over - bytes_transferred);
                    }
                    else
                    {
                        this->session->callback({}, this->session->initial_response);
                        this->listener.available(this->identifier);
                    }
                }
                else
                {
                    this->on_request_failure(err);
                }
            });
    }

    void connection_base::read_body()
    {
        asio::async_read(
            this->socket(),
            this->buffer,
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        std::istream stream(&this->buffer);
                        stream.read((char *)&this->session->initial_response.data[0], bytes_transferred);
                        this->read_body();
                    }
                    else
                    {
                        this->listener.available(this->identifier);
                    }
                }
                else
                {
                    this->on_request_failure(err);
                }
            });
    }

    void connection_base::read_event_stream(const std::shared_ptr<asio::streambuf> &buffer_ptr)
    {
        asio::async_read_until(
            socket(),
            *buffer_ptr,
            "\r\n\r\n",
            [this, buffer_ptr](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        std::istream stream(buffer_ptr.get());
                        std::string line;
                        // std::ostream out(&this->session->response.buffer);
                        while (std::getline(stream, line) && !line.empty() && !(line.back() == '\r' && line.size() == 1))
                        {
                            auto data_end = this->session->initial_response.data.end();
                            this->session->initial_response.data.insert(data_end, line.begin(), line.end());
                            std::string new_line("\n");
                            data_end = this->session->initial_response.data.end();
                            this->session->initial_response.data.insert(data_end, new_line.begin(), new_line.end());
                            // out.write(line.data(), static_cast<std::streamsize>(line.size() - (line.back() == '\r' ? 1 : 0)));
                            // out.put('\n');
                        }
                        this->session->callback({}, this->session->initial_response);
                        this->read_event_stream(buffer_ptr);
                        buffer_ptr->consume(buffer_ptr->size());
                    }
                    else
                    {
                        this->listener.available(this->identifier);
                    }
                }
                else
                {
                    this->on_request_failure(err);
                }
            });
    }

    void connection_base::read_chunk_size(const std::shared_ptr<asio::streambuf> &chunk_buffer)
    {
        asio::async_read_until(
            socket(),
            *chunk_buffer,
            "\r\n",
            [this, chunk_buffer](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err && bytes_transferred > 0)
                {
                    std::istream stream(chunk_buffer.get());
                    std::string line;
                    std::getline(stream, line);
                    std::size_t chunk_size = std::stoul(line, 0, 16);
                    if (chunk_size > 0)
                    {
                        auto additional_bytes = chunk_buffer->size() - bytes_transferred;
                        auto bytes_to_move = std::min<std::size_t>(chunk_size, additional_bytes);
                        if (bytes_to_move > 0)
                        {
                            chunk_buffer->consume(bytes_to_move);
                        }
                        if (chunk_size > additional_bytes)
                        {
                            this->read_chunk_content(chunk_size - additional_bytes, chunk_buffer);
                        }
                        else
                        {
                            this->clear_end_of_chunk(chunk_size, additional_bytes, stream, chunk_buffer);
                        }
                    }
                }
                else
                {
                    this->on_request_failure(err);
                }
            });
    }

    void connection_base::read_chunk_content(std::size_t bytes_to_transfer, const std::shared_ptr<asio::streambuf> &chunk_buffer)
    {
        asio::async_read(
            socket(),
            this->buffer,
            asio::transfer_exactly(bytes_to_transfer),
            [this, chunk_buffer](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        std::istream stream(&buffer);
                        std::vector<uint8_t> content(bytes_transferred);
                        stream.read((char *)&content[0], bytes_transferred);
                        this->session->initial_response.data.insert(this->session->initial_response.data.end(), content.begin(), content.end());
                        asio::streambuf null_buffer(2);
                        asio::async_read(
                            this->socket(),
                            null_buffer,
                            asio::transfer_exactly(2),
                            [this, chunk_buffer](const std::error_code &err, std::size_t bytes_transferred)
                            {
                                if (!err)
                                {
                                    this->read_chunk_size(chunk_buffer);
                                }
                                else
                                {
                                    this->on_request_failure(err);
                                }
                            });
                    }
                    else
                    {
                        this->listener.available(this->identifier);
                    }
                }
                else
                {
                    this->on_request_failure(err);
                }
            });
    }

    void connection_base::clear_end_of_chunk(std::size_t chunk_size, std::size_t additional_bytes, std::istream &stream, const std::shared_ptr<asio::streambuf> &chunk_buffer)
    {
        if (2 + chunk_size > additional_bytes)
        {
            if (2 + chunk_size - additional_bytes == 1)
            {
                stream.get();
                asio::streambuf null_buffer(2);
                asio::async_read(
                    socket(),
                    null_buffer,
                    asio::transfer_exactly(2 + chunk_size - additional_bytes),
                    [this, chunk_buffer](const std::error_code &err, std::size_t bytes_transferred)
                    {
                        if (!err)
                        {
                            this->read_chunk_size(chunk_buffer);
                        }
                        else
                        {
                            this->on_request_failure(err);
                        }
                    });
            }
            else
            {
                stream.get();
                stream.get();
                this->read_chunk_size(chunk_buffer);
            }
        }
    }

    void connection_base::on_request_failure(const std::error_code &error)
    {
        this->session->callback(error, {});
    }
    void connection_base::on_download_failure(const std::error_code &error)
    {
        this->download_ptr->callback(error, {});
    }

    void connection_base::on_upload_failure(const std::error_code &error)
    {
    }
}