#include <domain/images/http/secure_connection.h>
#include <domain/images/http/lifetime_listener.h>
#include <domain/images/http/download_destination.h>
#include <asio/connect.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace domain::images::http
{
    secure_connection::secure_connection(asio::io_context &context, asio::ssl::context &ssl_ctx, const std::string &identifier, lifetime_listener &listener) : connection_base(context, identifier, listener),
                                                                                                                                                               stream(context, ssl_ctx)
    {
        // ssl_ctx.set_verify_callback(asio::ssl::rfc2818_verification())
    }

    void secure_connection::download(const internal::uri &uri, const std::map<std::string, std::string> &headers, std::shared_ptr<download_destination> sink, report_callback callback)
    {
        request req("HEAD", uri, std::vector<uint8_t>{}, "", headers);
        logger->info("HOST: {} PORT: {}", req.host(), req.port());
        auto endpoints = resolve_endpoints(req.host(), req.port());
        logger->info("endpoint resolved");
        this->download_ptr = std::make_unique<file_download>();
        download_ptr->uri = uri;
        download_ptr->headers.insert(headers.begin(), headers.end());
        download_ptr->callback = std::move(callback);
        download_ptr->destination = std::move(sink);
        if (!download_ptr->destination->is_valid())
        {
            this->session->callback(std::make_error_code(std::errc::inappropriate_io_control_operation), {});
        }
        else
        {
            asio::async_connect(
                stream.next_layer(),
                endpoints,
                [this, req = std::move(req)](const std::error_code &err, asio::ip::tcp::endpoint)
                {
                    if (!err)
                    {
                        asio::ip::tcp::no_delay option(true);
                        this->socket().set_option(option);
                        this->stream.set_verify_callback(asio::ssl::host_name_verification{req.host()});
                        if (!SSL_set_tlsext_host_name(this->stream.native_handle(), req.host().c_str()))
                        {
                            logger->info("Failed to set SNI host name for SSL");
                        }
                        this->stream.async_handshake(
                            asio::ssl::stream_base::client,
                            [this, req = std::move(req)](const std::error_code &error)
                            {
                                if (error)
                                {
                                    this->session->callback(error, {});
                                }
                                else
                                {

                                    this->fetch_file_details(std::move(req));
                                }
                            });
                    }
                    else
                    {
                        this->session->callback(err, {});
                    }
                });
        }
    }
    void secure_connection::execute(const request &req, response_callback callback)
    {
        logger->info("HOST: {} PORT: {}", req.host(), req.port());
        auto endpoints = resolve_endpoints(req.host(), req.port());
        logger->info("endpoint resolved");
        session = std::make_unique<connection_session>();
        session->callback = std::move(callback);
        asio::async_connect(
            stream.next_layer(),
            endpoints,
            [this, req = std::move(req)](const std::error_code &err, asio::ip::tcp::endpoint)
            {
                if (!err)
                {
                    asio::ip::tcp::no_delay option(true);
                    this->socket().set_option(option);
                    this->stream.set_verify_callback(asio::ssl::host_name_verification{req.host()});
                    if (!SSL_set_tlsext_host_name(this->stream.native_handle(), req.host().c_str()))
                    {
                        logger->info("Failed to set SNI host name for SSL");
                    }
                    this->stream.async_handshake(
                        asio::ssl::stream_base::client,
                        [this, req = std::move(req)](const std::error_code &error)
                        {
                            if (error)
                            {
                                this->session->callback(error, {});
                            }
                            else
                            {
                                this->write_call(std::move(req));
                            }
                        });
                }
                else
                {
                    this->session->callback(err, {});
                }
            });
    }

    void secure_connection::upload(
        const internal::uri &uri,
        const std::map<std::string, std::string> &headers,
        const fs::path &file_path, upload_callback callback)
    {
    }

    asio::ip::tcp::socket &secure_connection::socket()
    {
        return stream.next_layer();
    }
    secure_connection::~secure_connection()
    {
        stream.shutdown();
        if (socket().is_open())
        {
            socket().cancel();
            socket().close();
        }
    }
}