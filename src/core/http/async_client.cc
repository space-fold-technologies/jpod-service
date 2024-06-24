#include <core/http/async_client.h>
#include <core/http/session.h>
#include <core/http/request.h>
#include <spdlog/spdlog.h>

namespace core::http
{
    async_client::async_client(session_provider provider, uint16_t retries) : provider(provider),
                                                                              retries(retries),
                                                                              session{},
                                                                              settings{},
                                                                              buffer(BUFFER_SIZE),
                                                                              connections{},
                                                                              logger(spdlog::get("jpod"))
    {
        /*Initialize user callbacks and settings */
        llhttp_settings_init(&settings);
        settings.on_message_begin = async_client::on_message_begin;
        settings.on_message_complete = async_client::on_message_complete;
        settings.on_body = async_client::on_body;
        llhttp_init(&http_parser, HTTP_RESPONSE, &settings);
        http_parser.data = this;
        llhttp_set_lenient_optional_lf_after_cr(&http_parser, 1);
    }
    void async_client::post(const request_details &details, response_callback callback)
    {
        if (auto result = compose_request(details, "POST"); !result)
        {
            callback(result.error(), {});
        }
        else
        {
            session.follow = false;
            session.method = "POST";
            session.retries = 0;
            session.initial_response = {};
            session.callback = std::move(callback);
            send_request(result.value());
        }
    }
    void async_client::put(const request_details &details, response_callback callback)
    {
        if (auto result = compose_request(details, "PUT"); !result)
        {
            callback(result.error(), {});
        }
        else
        {
            session.follow = false;
            session.method = "PUT";
            session.retries = 0;
            session.initial_response = {};
            session.callback = std::move(callback);
            send_request(result.value());
        }
    }
    void async_client::get(const std::string &path,
                           const http_headers &headers,
                           response_callback callback,
                           bool follow)
    {
        if (auto result = compose_request(path, headers, "GET"); !result)
        {
            callback(result.error(), {});
        }
        else
        {
            session.follow = follow;
            session.method = "GET";
            session.retries = 0;
            session.initial_response = {};
            session.headers.insert(headers.begin(), headers.end());
            session.callback = std::move(callback);
            send_request(result.value());
        }
    }
    void async_client::remove(
        const std::string &path,
        const http_headers &headers,
        response_callback callback)
    {
        if (auto result = compose_request(path, headers, "DELETE"); !result)
        {
            callback(result.error(), {});
        }
        else
        {
            session.follow = false;
            session.method = "DELETE";
            session.retries = 0;
            session.initial_response = {};
            session.callback = std::move(callback);
            send_request(result.value());
        }
    }
    void async_client::head(const std::string &path, const http_headers &headers, response_callback callback, bool follow)
    {
        if (auto result = compose_request(path, headers, "HEAD"); !result)
        {
            callback(result.error(), {});
        }
        else
        {
            session.follow = follow;
            session.method = "HEAD";
            session.retries = 0;
            session.initial_response = {};
            session.headers.insert(headers.begin(), headers.end());
            session.callback = std::move(callback);
            send_request(result.value());
        }
    }

    std::shared_ptr<http_session> async_client::connection(const internal::uri &uri)
    {

        if (connections.find(uri.full_host()) == connections.end())
        {
            connections.try_emplace(uri.full_host(), provider(uri.scheme, uri.host));
        }
        return connections.at(uri.full_host());
    }
    std::shared_ptr<http_session> async_client::connection(const std::string &id)
    {
        return connections.at(id);
    }
    void async_client::send_request(const request &req)
    {
        session.request.assign(req.content.begin(), req.content.end());
        session.id = req.url.full_host();
        connection(req.url)->connect(
            req.url.host,
            req.url.port,
            [this, req = std::move(req)](const std::error_code &error)
            {
                if (error)
                {
                    this->on_request_failure(error);
                }
                else
                {
                    connection(req.url)->async_write(
                        session.request,
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
                                    this->read_response_header();
                                }
                            }
                        });
                }
            });
    }

    void async_client::handle_redirect(const std::string &location)
    {
        std::error_code error{};
        if (auto result = compose_request(location, session.headers, session.method); !result)
        {
            on_request_failure(result.error());
        }
        else
        {
            session.initial_response = {};
            session.raw.clear();
            session.retries = 0;
            send_request(result.value());
        }
    }
    void async_client::retry_last_request()
    {
        auto current_connection = connection(session.id);
        session.initial_response = {};
        current_connection->reconnect(
            [this](const std::error_code &error)
            {
                if (error)
                {
                    on_response_failure(error);
                }
                else
                {
                    connection(session.id)->async_write(session.request, [this](const std::error_code &err, std::size_t bytes_transferred)
                                                        {
                            if (err)
                            {
                                logger->error("content not transferred: {}", err.message());
                                this->on_request_failure(err);
                            }
                            else if (bytes_transferred > 0)
                            {
                                this->read_response_header();
                                
                            } });
                }
            });
    }
    void async_client::read_response_header()
    {
        auto current_connection = connection(session.id);
        current_connection->read_header(
            this->buffer,
            [this](std::error_code error, std::size_t bytes_transferred)
            {
                if (error)
                {
                    if (error == asio::error::eof && session.retries < retries)
                    {
                        retry_last_request();
                    }
                    else
                    {
                        this->on_response_failure(error);
                    }
                }
                else if (bytes_transferred > 0)
                {
                    std::stringstream ss;
                    ss << &this->buffer;
                    std::string headers_contained = ss.str();
                    if (auto headers_end = headers_contained.find("\r\n\r\n"); headers_end != std::string::npos)
                    {
                        std::string exact_header_content = headers_contained.substr(0, headers_end + 4);
                        std::string remaining_content = headers_contained.substr(headers_end + 4);
                        std::size_t remaining_bytes = headers_contained.substr(headers_end + 4).size();
                        session.initial_response = parse_response(exact_header_content, error);
                        if (error)
                        {
                            this->on_response_failure(error);
                        }
                        else
                        {
                            if (session.initial_response.has_body())
                            {
                                auto part = ss.str();
                                session.raw.assign(part.begin(), part.end());
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
                                session.raw.assign(part.begin(), part.end());
                                buffer.consume(bytes_transferred);
                                read_next_chunk();
                            }
                            else if (session.initial_response.is_redirect())
                            {
                                if (!session.follow)
                                {
                                    session.callback({}, session.initial_response);
                                }
                                else
                                {
                                    buffer.consume(buffer.size());
                                    connection(session.id)->delay([this](const std::error_code &error)
                                                                  {
                                            if (error)
                                            {
                                                on_request_failure(error);
                                            }
                                            else
                                            {
                                                handle_redirect(session.initial_response.location());
                                            } });
                                }
                            }
                        }
                    }
                }
            });
    }
    void async_client::read_body(std::size_t bytes_left_over)
    {
        auto current_connection = connection(session.id);
        current_connection->read_exactly(
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
                        session.raw.insert(session.raw.end(), content.begin(), content.end());
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
    void async_client::read_body()
    {
        auto current_connection = connection(session.id);
        current_connection->read(
            this->buffer,
            [this](const std::error_code &error, std::size_t bytes_transferred)
            {
                if (!error)
                {
                    if (bytes_transferred > 0)
                    {
                        std::istream stream(&this->buffer);
                        std::vector<uint8_t> content(bytes_transferred);
                        stream.read((char *)&content[0], bytes_transferred);
                        session.raw.insert(session.raw.end(), content.begin(), content.end());
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
                    if (error == asio::error::eof)
                    {
                        buffer.consume(buffer.size());
                        parse_chunks();
                    }
                    else
                    {
                        this->on_request_failure(error);
                    }
                }
            });
    }
    void async_client::read_next_chunk()
    {
        auto current_connection = connection(session.id);
        current_connection->read_until(
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
                else if (bytes_transferred > 0)
                {
                    std::istream stream(&buffer);
                    std::vector<uint8_t> content(bytes_transferred);
                    stream.read((char *)&content[0], bytes_transferred);
                    session.raw.insert(session.raw.end(), content.begin(), content.end());
                    buffer.consume(bytes_transferred);
                    read_next_chunk();
                }
            });
    }
    void async_client::parse_chunks()
    {
        std::string content(session.raw.begin(), session.raw.end());
        if (auto result = llhttp_execute(&http_parser, content.c_str(), content.size()); result != HPE_OK)
        {
            logger->error("parser nxt-cnk error: ERR: {} REASON: {}", llhttp_errno_name(result), http_parser.reason);
        }
    }

    void async_client::on_request_failure(const std::error_code &error)
    {
        session.callback(error, {});
    }
    void async_client::on_response_failure(const std::error_code &error)
    {
        session.callback(error, {});
    }

    int async_client::on_message_begin(llhttp_t *parser)
    {
        auto self = static_cast<async_client *>(parser->data);
        self->logger->trace("begin body");
        return 0;
    }

    int async_client::on_body(llhttp_t *parser, char const *data, size_t length)
    {
        auto self = static_cast<async_client *>(parser->data);
        self->session.initial_response.data.reserve(length);
        std::string content(data, length);
        self->session.initial_response.data.insert(self->session.initial_response.data.end(), content.begin(), content.end());
        return 0;
    }

    int async_client::on_message_complete(llhttp_t *parser)
    {
        auto self = static_cast<async_client *>(parser->data);
        self->logger->trace("completed");
        self->buffer.consume(self->buffer.size());
        self->session.callback({}, self->session.initial_response);
        return 0;
    }
    async_client::~async_client()
    {
        if(!connections.empty())
        {
            connections.clear();
        }
        if(!session.initial_response.headers.empty())
        {
            session.initial_response.headers.clear();
        }
        if(!session.initial_response.data.empty())
        {
            session.initial_response.data.clear();
        }
    }
}