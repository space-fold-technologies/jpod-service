#include <core/http/rest_client.h>
#include <core/http/session.h>
#include <core/http/utility.h>
#include <core/http/uri.h>
#include <spdlog/spdlog.h>

namespace core::http
{
    rest_client::rest_client(session_provider provider) : provider(provider),
                                                          buffer(1024 * 6),
                                                          connection(nullptr),
                                                          http_parser{},
                                                          settings{},
                                                          retry_count(0),
                                                          retry_max(30),
                                                          follow(false),
                                                          logger(spdlog::get("jpod"))
    {
        /*Initialize user callbacks and settings */
        llhttp_settings_init(&settings);
        settings.on_message_begin = rest_client::on_message_begin;
        settings.on_message_complete = rest_client::on_message_complete;
        settings.on_body = rest_client::on_body;
        llhttp_init(&http_parser, HTTP_RESPONSE, &settings);
        http_parser.data = this;
        llhttp_set_lenient_optional_lf_after_cr(&http_parser, 1);
    }
    void rest_client::post(const request_details &details, response_callback callback)
    {
        std::vector<uint8_t> content;
        internal::uri url;
        if (auto error = compose_request(content, url, details, "POST"); error)
        {
            callback(error, {});
        }
        else
        {
            session.callback = std::move(callback);
            write_call(content, url);
        }
    }
    void rest_client::put(const request_details &details, response_callback callback)
    {
        std::vector<uint8_t> content;
        internal::uri url;
        if (auto error = compose_request(content, url, details, "PUT"); error)
        {
            callback(error, {});
        }
        else
        {
            session.callback = std::move(callback);
            write_call(content, url);
        }
    }
    void rest_client::get(const std::string &path, const http_headers &headers, response_callback callback, bool follow)
    {
        std::vector<uint8_t> content;
        internal::uri url;
        this->follow = follow;
        if (auto error = compose_request(content, url, path, headers, "GET"); error)
        {
            callback(error, {});
        }
        else
        {
            session.callback = std::move(callback);
            write_call(content, url);
        }
    }
    void rest_client::remove(const std::string &path, const http_headers &headers, response_callback callback)
    {
        std::vector<uint8_t> content;
        internal::uri url;
        if (auto error = compose_request(content, url, path, headers, "DELETE"); error)
        {
            callback(error, {});
        }
        else
        {
            session.callback = std::move(callback);
            write_call(content, url);
        }
    }

    void rest_client::head(const std::string &path, const http_headers &headers, response_callback callback, bool follow)
    {
        std::vector<uint8_t> content;
        internal::uri url;
        this->follow = follow;
        if (auto error = compose_request(content, url, path, headers, "HEAD"); error)
        {
            callback(error, {});
        }
        else
        {
            session.callback = std::move(callback);
            write_call(content, url);
        }
    }

    void rest_client::shutdown()
    {
        if (connection)
        {
            connection->shutdown();
        }
    }

    void rest_client::handle_redirect(const std::string &location)
    {
        std::vector<uint8_t> content;
        internal::uri url;
        std::map<std::string, std::string> headers;
        if (auto error = compose_request(content, url, location, headers, "GET"); error)
        {
            on_request_failure(error);
        }
        else
        {

            last_request.clear();
            last_request.assign(content.begin(), content.end());
            if (url.host != last_host)
            {
                last_host = url.host;
                connection.reset();
                connection = provider(url.scheme, url.host);
                connection->connect(
                    url.host,
                    url.port,
                    [this, content = std::move(content)](const std::error_code &error)
                    {
                        if (error)
                        {
                            this->on_request_failure(error);
                        }
                        else
                        {
                            last_request.clear();
                            last_request.assign(content.begin(), content.end());
                            connection->async_write(
                                last_request,
                                [this](const std::error_code &err, std::size_t bytes_transferred)
                                {
                                    if (err)
                                    {
                                        logger->error("content not transferred: {}", err.message());
                                        this->on_request_failure(err);
                                    }
                                    else
                                    {
                                        if (bytes_transferred > 0)
                                        {
                                            logger->info("transferred : {} bytes", bytes_transferred);
                                            this->read_response_header();
                                        }
                                    }
                                });
                        }
                    });
            }
            else
            {
                handle_retry();
            }
        }
    }

    void rest_client::handle_retry()
    {
        // we try reading again
        connection->reconnect([this](const std::error_code &error)
                              {
                                if(error)
                                {
                                    on_response_failure(error);
                                } else 
                                {
                                    std::string content(last_request.begin(), last_request.end());
                                    connection->async_write(
                                            last_request,
                                            [this](const std::error_code &err, std::size_t bytes_transferred)
                                            {
                                                if (err)
                                                {
                                                    logger->error("content not transferred: {}", err.message());
                                                    this->on_request_failure(err);
                                                }
                                                else
                                                {
                                                    if (bytes_transferred > 0)
                                                    {
                                                        logger->info("transferred : {} bytes", bytes_transferred);
                                                        this->read_response_header();
                                                    }
                                                }
                                            });
                                } });
    }

    void rest_client::write_call(const std::vector<uint8_t> &content, const internal::uri &url)
    {
        std::string outgoing(content.begin(), content.end());
        if (!connection || !connection->is_scheme_matched(url.scheme) || url.host != last_host)
        {
            if (connection)
            {
                connection->shutdown();
            }
            connection.reset();
            connection = provider(url.scheme, url.host);
            last_host = url.host;
        }
        if (!raw_response.empty())
        {
            raw_response.clear();
        }

        logger->info("CURRENT HOST: {}", last_host);
        connection->connect(
            url.host,
            url.port,
            [this, content = std::move(content)](const std::error_code &error)
            {
                if (error)
                {
                    this->on_request_failure(error);
                }
                else
                {
                    last_request.clear();
                    last_request.assign(content.begin(), content.end());
                    connection->async_write(
                        last_request,
                        [this](const std::error_code &err, std::size_t bytes_transferred)
                        {
                            if (err)
                            {
                                logger->error("content not transferred: {}", err.message());
                                this->on_request_failure(err);
                            }
                            else
                            {
                                if (bytes_transferred > 0)
                                {
                                    logger->info("transferred : {} bytes", bytes_transferred);
                                    this->read_response_header();
                                }
                            }
                        });
                }
            });
    }
    std::error_code rest_client::compose_request(
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
    std::error_code rest_client::compose_request(std::vector<uint8_t> &content, internal::uri &url, const request_details &details, std::string method)
    {
        std::error_code error{};
        if (url = internal::parse_url(details.path, error); error)
        {
            return error;
        }
        else
        {
            auto header_line = fmt::format("{} {} HTTP/1.1\r\n"
                                           "Host: {}\r\n"
                                           "User-Agent: jpod client\r\n"
                                           "Content-Type: {}\r\n"
                                           "Content-Length: {}\r\n",
                                           method,
                                           url.path,
                                           url.full_host(),
                                           details.content_type,
                                           details.content.size());
            auto headers = details.headers;
            std::for_each(headers.begin(), headers.end(), [&header_line](const auto &entry)
                          { header_line += fmt::format("{}: {}\r\n", entry.first, entry.second); });
            header_line += "\r\n";
            content.reserve(header_line.size() + details.content.size());
            content.assign(header_line.begin(), header_line.end());
            if (!details.content.empty())
            {
                content.insert(content.end(), details.content.begin(), details.content.end());
            }
            return {};
        }
    }
    void rest_client::on_request_failure(const std::error_code &error)
    {
        session.callback(error, {});
    }
    void rest_client::on_response_failure(const std::error_code &error)
    {
        session.callback(error, {});
    }
    void rest_client::read_response_header()
    {
        connection->read_header(
            this->buffer,
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
                            std::string remaining_content = headers_contained.substr(headers_end + 4);
                            std::size_t remaining_bytes = headers_contained.substr(headers_end + 4).size();

                            std::error_code error{};
                            if (session.initial_response = parse_response(exact_header_content, error); error)
                            {
                                logger->error("parsing header content failed");
                                this->on_response_failure(error);
                            }
                            else
                            {
                                if (session.initial_response.has_body())
                                {
                                    auto part = ss.str();
                                    raw_response.assign(part.begin(), part.end());
                                    std::size_t content_length = session.initial_response.content_length();
                                    this->buffer.consume(bytes_transferred);
                                    this->read_body(remaining_bytes < content_length ? content_length - remaining_bytes : content_length);
                                }
                                else if (session.initial_response.is_closed())
                                {
                                    this->buffer.consume(bytes_transferred);
                                    this->read_body();
                                }
                                else if (session.initial_response.is_chunked())
                                {
                                    auto part = ss.str();
                                    raw_response.assign(part.begin(), part.end());
                                    this->buffer.consume(bytes_transferred);
                                    read_next_chunk();
                                }
                                else if (session.initial_response.is_event_stream())
                                {
                                    auto events_stream_buffer = std::make_shared<asio::streambuf>(std::numeric_limits<std::size_t>::max());
                                    events_stream_buffer->commit(asio::buffer_copy(events_stream_buffer->prepare(this->buffer.size()), this->buffer.data()));
                                    this->buffer.consume(this->buffer.size());
                                    this->read_event_stream(events_stream_buffer);
                                }
                                else if (session.initial_response.is_redirect())
                                {
                                    if (!follow)
                                    {
                                        logger->info("not handling re-direct");
                                        session.callback({}, session.initial_response);
                                    }
                                    else
                                    {
                                        logger->info("been asked to re-direct");
                                        buffer.consume(bytes_transferred);
                                        connection->delay(
                                            [this, location = session.initial_response.location()](const std::error_code &error)
                                            {
                                                if (error)
                                                {
                                                    on_request_failure(error);
                                                }
                                                else
                                                {
                                                    logger->info("redirecting now");
                                                    handle_redirect(location);
                                                }
                                            });
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (err == asio::error::eof)
                    {
                        if (retry_count++ < retry_max)
                        {
                            handle_retry();
                        }
                        else
                        {
                            this->on_response_failure(err);
                        }
                    }
                    else
                    {
                        retry_count = 0;
                        logger->error("initial header read failed: {}", err.message());
                        this->on_response_failure(err);
                    }
                }
            });
    }

    void rest_client::read_body(std::size_t bytes_left_over)
    {
        connection->read_exactly(
            this->buffer,
            bytes_left_over,
            [this, bytes_left_over](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        std::istream stream(&this->buffer);
                        std::vector<uint8_t> content(bytes_transferred);
                        stream.read((char *)&content[0], bytes_transferred);
                        raw_response.insert(raw_response.end(), content.begin(), content.end());
                        buffer.consume(bytes_transferred);
                        this->read_body(bytes_left_over - bytes_transferred);
                    }
                    else
                    {
                        buffer.consume(buffer.size());
                        parse_chunks();
                    }
                }
                else
                {
                    if (err == asio::error::eof)
                    {
                        parse_chunks();
                    }
                    else
                    {
                        this->on_request_failure(err);
                    }
                }
            });
    }

    void rest_client::read_body()
    {
        connection->read(
            this->buffer,
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        std::istream stream(&this->buffer);
                        std::vector<uint8_t> content(bytes_transferred);
                        stream.read((char *)&content[0], bytes_transferred);
                        raw_response.insert(raw_response.end(), content.begin(), content.end());
                        buffer.consume(bytes_transferred);
                        this->read_body();
                    }
                    else
                    {
                        parse_chunks();
                    }
                }
                else
                {
                    if (err == asio::error::eof)
                    {
                        buffer.consume(buffer.size());
                        parse_chunks();
                    }
                    else
                    {
                        this->on_request_failure(err);
                    }
                }
            });
    }

    void rest_client::read_event_stream(const std::shared_ptr<asio::streambuf> &buffer_ptr)
    {
        connection->read_until(
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
                            auto data_end = session.initial_response.data.end();
                            session.initial_response.data.insert(data_end, line.begin(), line.end());
                            std::string new_line("\n");
                            data_end = session.initial_response.data.end();
                            session.initial_response.data.insert(data_end, new_line.begin(), new_line.end());
                            // out.write(line.data(), static_cast<std::streamsize>(line.size() - (line.back() == '\r' ? 1 : 0)));
                            // out.put('\n');
                        }
                        session.callback({}, session.initial_response);
                        this->read_event_stream(buffer_ptr);
                        buffer_ptr->consume(buffer_ptr->size());
                    }
                }
                else
                {
                    this->on_request_failure(err);
                }
            });
    }

    void rest_client::read_next_chunk()
    {
        connection->read_until(
            buffer,
            "\r\n\r\n",
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (err)
                {
                    if (err == asio::error::eof)
                    {
                        parse_chunks();
                        llhttp_reset(&http_parser);
                    }
                    else
                    {
                        this->on_request_failure(err);
                    }
                }
                else
                {
                    if (bytes_transferred > 0)
                    {
                        std::istream stream(&buffer);
                        std::vector<uint8_t> content(bytes_transferred);
                        stream.read((char *)&content[0], bytes_transferred);
                        raw_response.insert(raw_response.end(), content.begin(), content.end());
                        buffer.consume(bytes_transferred);
                        read_next_chunk();
                    }
                }
            });
    }
    void rest_client::parse_chunks()
    {
        std::string content(raw_response.begin(), raw_response.end());
        if (auto result = llhttp_execute(&http_parser, content.c_str(), content.size()); result != HPE_OK)
        {
            logger->error("parser nxt-cnk error: ERR: {} REASON: {}", llhttp_errno_name(result), http_parser.reason);
        }
    }
    int rest_client::on_message_begin(llhttp_t *parser)
    {
        auto self = static_cast<rest_client *>(parser->data);
        self->logger->trace("begin body");
        return 0;
    }

    int rest_client::on_body(llhttp_t *parser, char const *data, size_t length)
    {
        auto self = static_cast<rest_client *>(parser->data);
        self->session.initial_response.data.reserve(length);
        std::string content(data, length);
        self->session.initial_response.data.insert(self->session.initial_response.data.end(), content.begin(), content.end());
        return 0;
    }

    int rest_client::on_message_complete(llhttp_t *parser)
    {
        auto self = static_cast<rest_client *>(parser->data);
        self->logger->info("completed");
        self->buffer.consume(self->buffer.size());
        self->session.callback({}, self->session.initial_response);
        return 0;
    }

    rest_client::~rest_client()
    {
        // we will probably have both variations of the sockets or just a helper to nuke the reference to the socket
        if (connection)
        {
            connection.reset();
        }
    }
}