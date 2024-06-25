#include <core/http/file_transfer_client.h>
#include <core/http/file_transfer_payloads.h>
#include <core/http/download_destination.h>
#include <core/http/response.h>
#include <core/http/session.h>
#include <asio/ssl/error.hpp>
#include <asio/read.hpp>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace core::http
{
    file_transfer_client::file_transfer_client(asio::io_context &context, session_provider provider) : context(context),
                                                                                                       provider(provider),
                                                                                                       _download(nullptr),
                                                                                                       _upload(nullptr),
                                                                                                       connection(nullptr),
                                                                                                       logger(spdlog::get("jpod"))
    {
    }
    void file_transfer_client::download(const download_request &request, report_callback callback)
    {
        // we need something that will be used to initiate the request
        std::error_code error{};
        if (auto url = internal::parse_url(request.url, error); error)
        {
            callback(error, {});
        }
        else
        {
            connection = provider(url.scheme, url.host);
            _download = std::make_unique<file_download>();
            _download->name = std::move(request.name);
            _download->uri = url;
            _download->headers = std::move(request.headers);
            _download->callback = std::move(callback);
            _download->destination = std::move(request.sink);
            _download->media_type = request.media_type;

            if (!_download->destination->is_valid())
            {
                on_download_failure(std::make_error_code(std::errc::inappropriate_io_control_operation));
            }
            else if (!connection->is_connected())
            {
                connection->connect(
                    _download->uri.host,
                    _download->uri.port,
                    [this](const std::error_code &error)
                    {
                        if (error)
                        {
                            on_download_failure(error);
                        }
                        else
                        {
                            fetch_file_details();
                        }
                    });
            }
            else
            {
                fetch_file_details();
            }
        }
    }
    void file_transfer_client::upload(const upload_request &request, upload_callback callback)
    {
        std::error_code error{};
        if (auto url = internal::parse_url(request.url, error); error)
        {
            callback(error, {});
        }
        else
        {
            if (!connection || !connection->is_scheme_matched(url.scheme))
            {
                connection.reset();
                connection = provider(url.scheme, url.host);
            }
            connection->connect(
                url.host,
                url.port,
                [this, url = std::move(url), request = std::move(request), cb = std::move(callback)](const std::error_code &error)
                {
                    if (error)
                    {
                        cb(error, {});
                    }
                    else
                    {
                        _upload = std::make_unique<file_upload>();
                        _upload->file_name = request.file_name;
                        _upload->uri = std::move(url);
                        _upload->callback = std::move(cb);
                        _upload->path = request.file_path;
                        _upload->headers = std::move(request.headers);
                        open_file();
                    }
                });
        }
    }
    void file_transfer_client::fetch_file_details()
    {
        std::vector<uint8_t> request_data;
        std::map<std::string, std::string> headers{_download->headers};
        headers.emplace("Referer", _download->uri.original);
        headers.emplace("Range", "bytes=0-");
        headers.emplace("Accept", _download->media_type);

        if (auto error = compose_request(request_data, _download->uri, headers, "HEAD"); error)
        {
            on_download_failure(error);
        }
        else
        {
            std::string content(request_data.begin(), request_data.end());
            connection->async_write(
                request_data,
                [this, headers = std::move(headers)](const std::error_code &error, std::size_t bytes_transferred)
                {
                    if (error)
                    {
                        on_download_failure(error);
                    }
                    else
                    {
                        on_server_response(headers, download_state::RANGE_QUERY, "HEAD");
                    }
                });
        }
    }
    void file_transfer_client::on_server_response(const http_headers &headers, download_state state, const std::string &method)
    {
        connection->read_header(
            _download->buffer,
            [this, headers = std::move(headers), method = std::move(method), state](std::error_code error, std::size_t bytes_transferred)
            {
                if (error)
                {
                    if (error.category() == asio::ssl::error::get_stream_category())
                    {
                        handle_retry(headers, state, method);
                    }
                    else
                    {
                        on_download_failure(error);
                    }
                }
                else if (bytes_transferred > 0)
                {
                    std::stringstream ss;
                    ss << &_download->buffer;
                    std::string headers_contained = ss.str();
                    if (auto headers_end = headers_contained.find("\r\n\r\n"); headers_end != std::string::npos)
                    {
                        std::string exact_header_content = headers_contained.substr(0, headers_end + 4);
                        _download->buffer.consume(exact_header_content.length());
                        std::error_code error{};
                        if (auto response = parse_response(exact_header_content, error); error)
                        {
                            logger->error("parsing header content failed");
                            this->on_download_failure(error);
                        }
                        else if (response.is_redirect())
                        {
                            handle_redirect(headers, download_state::RANGE_QUERY, response.location(), method);
                        }
                        else if (response.has_partial_body() && method == "GET")
                        {
                            auto content_range = response.content_range();
                            auto unit = _download->status.unit;
                            auto range_captured = content_range->replace(content_range->find(unit), unit.size(), "");
                            auto range = range_captured.substr(0, range_captured.find("/"));
                            _download->status.start = std::stoull(range.substr(0, range.find("-")));
                            _download->status.end = std::stoull(range.substr(range.find("-") + 1));
                            _download->status.total = std::stoull(range_captured.substr(range_captured.find("/") + 1));
                            auto content_length = response.content_length();
                            _download->status.current += content_length;
                            _download->chunk_size = content_length;
                            std::string excess = headers_contained.substr(headers_end + 4);
                            std::size_t remaining_bytes = excess.size();
                            if (remaining_bytes > 0)
                            {
                                auto content = std::vector<uint8_t>(excess.begin(), excess.end());
                                _download->destination->write(content);
                                _download->buffer.consume(content.size());
                            }
                            size_t content_to_fetch = content_length - remaining_bytes;
                            read_partial_content(content_to_fetch);
                        }
                        else if (response.accepts_partial_request() && method == "HEAD")
                        {
                            download_status status{};
                            status.total = response.content_length();
                            status.unit = response.accepts_ranges();
                            status.current = 0L;
                            _download->status = std::move(status);
                            _download->callback({}, _download->status);
                            std::size_t chunk_size = std::min<std::size_t>(
                                _download->destination->chunk_size(),
                                _download->status.total);
                            chunk_request request{};
                            request.start = 0;
                            request.end = chunk_size - 1;
                            request.size = chunk_size;
                            _download->current_request.emplace(request);
                            request_partial_content();
                        }
                    }
                }
                else
                {
                    logger->info("no response from server");
                    // maybe retry from here
                }
            });
    }
    void file_transfer_client::handle_redirect(const http_headers &headers, download_state state, const std::string &location, const std::string &method)
    {
        // in the event that the connection re-directs to a new host, you might need to make that your primary
        std::vector<uint8_t> content;
        internal::uri url;
        if (auto error = compose_request(content, url, location, headers, method); error)
        {
            on_download_failure(error);
        }
        else
        {
            connection->shutdown();
            connection.reset();
            connection = provider(url.scheme, url.host);
            _download->uri = url;
            connection->connect(
                url.host,
                url.port,
                [this, content = std::move(content), headers = std::move(headers), method = std::move(method), state](const std::error_code &error)
                {
                    if (error)
                    {
                        this->on_download_failure(error);
                    }
                    else
                    {
                        connection->async_write(
                            content,
                            [this, headers = std::move(headers), method = std::move(method), state](const std::error_code &err, std::size_t bytes_transferred)
                            {
                                if (err)
                                {
                                    logger->error("content not transferred: {}", err.message());
                                    this->on_download_failure(err);
                                }
                                else
                                {
                                    if (bytes_transferred > 0)
                                    {
                                        this->on_server_response(headers, state, method);
                                    }
                                }
                            });
                    }
                });
        }
    }
    void file_transfer_client::handle_retry(const http_headers &headers, download_state state, const std::string &method)
    {
        connection->reconnect([this, state, method](const std::error_code &error)
                              {
            if(error)
            {
                on_download_failure(error);
            } else 
            {
                switch(state)
                {
                    case download_state::CHUNK_DOWNLOAD:
                        {
                            request_partial_content();
                        break;
                        }
                    case download_state::RANGE_QUERY:
                        {
                            fetch_file_details();
                        break;
                        }
                    default:
                    logger->info("can't be handled");
                    break;
                };
            } });
    }
    void file_transfer_client::request_partial_content()
    {
        std::map<std::string, std::string> headers{_download->headers};
        if (!_download->current_request)
        {
            logger->trace("no current request details set");
        }
        auto start = _download->current_request->start;
        auto end = _download->current_request->end;
        headers.try_emplace("Range", fmt::format("{}={}-{}", _download->status.unit, start, end));
        headers.try_emplace("Connection", "Keep-Alive");
        headers.emplace("Accept", _download->media_type);
        std::vector<uint8_t> request_data;
        if (auto error = compose_request(request_data, _download->uri, headers, "GET"); error)
        {
            on_download_failure(error);
        }
        else
        {
            connection->async_write(
                request_data,
                [this, headers = std::move(headers)](const std::error_code &error, std::size_t bytes_transferred)
                {
                    if (error)
                    {
                        if (error.category() == asio::ssl::error::get_stream_category())
                        {
                            connection->reconnect(
                                [this](const std::error_code &error)
                                {
                                    if (error)
                                    {
                                        on_download_failure(error);
                                    }
                                    else
                                    {
                                        request_partial_content();
                                    }
                                });
                        }
                        on_download_failure(error);
                    }
                    else if (bytes_transferred > 0)
                    {
                        on_server_response(headers, download_state::CHUNK_FETCH, "GET");
                    }
                    else
                    {
                        request_partial_content();
                    }
                });
        }
    }

    void file_transfer_client::read_partial_content(std::size_t bytes_to_transfer)
    {
        if (bytes_to_transfer == 0)
        {
            // We are done
            _download->status.complete = true;
            _download->callback({}, _download->status);
        }
        else
        {
            connection->read_exactly(
                _download->buffer,
                bytes_to_transfer,
                [this, bytes_to_transfer](const std::error_code &error, std::size_t bytes_transferred)
                {
                    if (error)
                    {
                        this->on_download_failure(error);
                    }
                    else if (bytes_to_transfer > 0)
                    {
                        std::istream stream(&_download->buffer);
                        std::vector<uint8_t> remainder(bytes_transferred);
                        stream.read((char *)&remainder[0], bytes_transferred);
                        _download->destination->write(remainder);
                        _download->buffer.consume(bytes_transferred);
                        if (_download->status.current < _download->status.total)
                        {
                            auto end_index = _download->status.total - 1;
                            auto next_start = _download->status.end + 1;
                            auto next_chunk_size = _download->chunk_size + next_start < end_index ? _download->chunk_size : (end_index - next_start) + 1;
                            auto next_end = (next_start + next_chunk_size) - 1;
                            _download->status.complete = false;
                            _download->callback({}, _download->status);
                            chunk_request request{next_start, next_end, bytes_to_transfer};
                            _download->current_request.emplace(request);
                            request_partial_content();
                        }
                        else
                        {
                            logger->info("done with current download");
                            // We are done
                            _download->status.complete = true;
                            _download->callback({}, _download->status);
                        }
                    }
                });
        }
    }

    void file_transfer_client::open_file()
    {
        if (auto fd = open(_upload->path.generic_string().c_str(), O_RDONLY); fd == -1)
        {
            on_upload_failure(std::error_code{errno, std::system_category()});
        }
        else
        {
            _upload->file_descriptor = fd;
            _upload->file_stream = std::make_unique<asio::posix::stream_descriptor>(context, fd);
            register_file_details();
        }
    }
    void file_transfer_client::register_file_details()
    {
        // place a record of the file details to the server
        std::map<std::string, std::string> headers(_upload->headers);
        std::vector<uint8_t> content;
        if (auto error = compose_request(content, _upload->uri, headers, "POST"); error)
        {
            on_upload_failure(error);
        }
        else
        {
            read_file_chunk();
        }
    }
    void file_transfer_client::read_file_chunk()
    {
        asio::async_read(
            *_upload->file_stream,
            asio::buffer(_upload->file_chunk_buffer),
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (err)
                {
                    on_upload_failure(err);
                }
                else if (bytes_transferred > 0)
                {
                    send_file_chunk(bytes_transferred);
                }
                else
                {
                    auto status = _upload->status;
                    status.complete = true;
                    _upload->callback({}, status);
                    _upload->file_stream->close();
                    close(_upload->file_descriptor);
                }
            });
    }
    void file_transfer_client::send_file_chunk(std::size_t bytes_to_transfer)
    {
        // auto status = _upload->status;
        // std::size_t start = status.end + 1;
        // std::size_t end = status.end + bytes_to_transfer;
        std::map<std::string, std::string> headers(_upload->headers);
        headers.try_emplace("Content-Range", fmt::format("{}={}-{}", _upload->status.unit, _upload->status.start, _upload->status.end));
        headers.try_emplace("Content-Length", fmt::format("{}", _upload->status.total));
        std::vector<uint8_t> payload;
        if (auto error = compose_request(payload, _upload->uri, headers, "PUT"); error)
        {
            on_upload_failure(error);
        }
        else
        {
            payload.insert(payload.end(), _upload->file_chunk_buffer.begin(), _upload->file_chunk_buffer.end());
            connection->async_write(
                payload,
                [this, bytes_to_transfer](const std::error_code &err, std::size_t bytes_transferred)
                {
                    if (!err && bytes_transferred > 0)
                    {
                        auto previous_status = _upload->status;
                        _upload->status.current += bytes_to_transfer;
                        _upload->status.start = previous_status.end + 1;
                        _upload->status.end = previous_status.end + bytes_to_transfer;
                        _upload->callback({}, _upload->status);
                        read_file_chunk();
                    }
                    else
                    {
                        this->on_upload_failure(err);
                    }
                });
        }
    }
    std::error_code file_transfer_client::compose_request(
        std::vector<uint8_t> &content,
        const internal::uri &url,
        const http_headers &headers,
        std::string method)
    {
        auto header_line = fmt::format("{} {} HTTP/1.1\r\n"
                                       "Host: {}\r\n"
                                       "User-Agent: jpod client\r\n",
                                       method,
                                       url.full_path(),
                                       url.full_host());
        std::for_each(headers.begin(), headers.end(), [&header_line](const auto &entry)
                      { header_line += fmt::format("{}: {}\r\n", entry.first, entry.second); });
        header_line += "\r\n";
        content.reserve(header_line.size());
        content.assign(header_line.begin(), header_line.end());
        return {};
    }
    std::error_code file_transfer_client::compose_request(
        std::vector<uint8_t> &content,
        internal::uri &url,
        const std::string &path,
        const http_headers &headers,
        std::string method)
    {
        std::error_code error{};
        if (url = internal::parse_url(path, error); error)
        {
            return error;
        }
        else
        {
            auto header_line = fmt::format("{} {} HTTP/1.1\r\n"
                                           "Host: {}\r\n"
                                           "User-Agent: jpod client\r\n",
                                           method,
                                           url.full_path(),
                                           url.full_host());
            std::for_each(headers.begin(), headers.end(), [&header_line](const auto &entry)
                          { header_line += fmt::format("{}: {}\r\n", entry.first, entry.second); });
            header_line += "\r\n";
            content.reserve(header_line.size());
            content.assign(header_line.begin(), header_line.end());
            return {};
        }
    }
    void file_transfer_client::on_download_failure(const std::error_code &error)
    {
        if (_download)
        {
            _download->callback(error, {});
        }
    }
    void file_transfer_client::on_upload_failure(const std::error_code &error)
    {
        if (_upload)
        {
            _upload->callback(error, {});
        }
    }
    file_transfer_client::~file_transfer_client()
    {
        if (connection)
        {
            connection.reset();
        }
    }
}