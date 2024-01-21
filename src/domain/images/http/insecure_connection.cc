#include <domain/images/http/insecure_connection.h>
#include <domain/images/http/lifetime_listener.h>
#include <domain/images/http/download_destination.h>
#include <asio/connect.hpp>
#include <memory>
#include <spdlog/spdlog.h>

namespace domain::images::http
{
    insecure_connection::insecure_connection(asio::io_context &context, const std::string &identifier, lifetime_listener &listener) : connection_base(context, identifier, listener),
                                                                                                                                      _socket(context)
    {
    }
    void insecure_connection::download(const internal::uri &uri, const std::map<std::string, std::string> &headers, std::shared_ptr<download_destination> sink, report_callback callback)
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
                _socket,
                endpoints,
                [this, req = std::move(req)](const std::error_code &err, asio::ip::tcp::endpoint)
                {
                    if (!err)
                    {
                        asio::ip::tcp::no_delay option(true);
                        this->socket().set_option(option);
                        this->fetch_file_details(std::move(req));
                    }
                    else
                    {
                        this->session->callback(err, {});
                    }
                });
        }
    }
    void insecure_connection::execute(const request &req, response_callback callback)
    {
        logger->info("HOST: {} PORT: {}", req.host(), req.port());
        auto endpoints = resolve_endpoints(req.host(), req.port());
        logger->info("endpoint resolved");
        session = std::make_unique<connection_session>();
        session->callback = std::move(callback);
        asio::async_connect(
            _socket,
            endpoints,
            [this, req = std::move(req)](const std::error_code &err, asio::ip::tcp::endpoint)
            {
                if (!err)
                {
                    asio::ip::tcp::no_delay option(true);
                    this->socket().set_option(option);
                    this->write_call(std::move(req));
                }
                else
                {
                    this->session->callback(err, {});
                }
            });
    }
    void insecure_connection::upload(
        const internal::uri &uri,
        const std::map<std::string, std::string> &headers,
        const fs::path &file_path, upload_callback callback)
    {
    }
    asio::ip::tcp::socket &insecure_connection::socket()
    {
        return _socket;
    }
    insecure_connection::~insecure_connection()
    {
        if (socket().is_open())
        {
            socket().cancel();
            socket().close();
        }
    }
}