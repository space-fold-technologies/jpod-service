#include <core/http/insecure_session.h>

#include <asio/io_context.hpp>
#include <asio/connect.hpp>
#include <asio/write.hpp>
#include <asio/read.hpp>
#include <core/http/utility.h>
#include <arpa/inet.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace core::http
{
    insecure_http_session::insecure_http_session(asio::io_context &context) : resolver(context),
                                                                              timer(context),
                                                                              scheme("http"),
                                                                              host(""),
                                                                              port(80),
                                                                              connected(false),
                                                                              logger(spdlog::get("jpod"))
    {
        socket.emplace(context);
    }

    void insecure_http_session::connect(const std::string &host, uint16_t port, initialization_callback callback)
    {
        if (!connected)
        {
            logger->info("connecting to: {}", host);
            resolve_async(
                host,
                port,
                [this, host = std::move(host), port, cb = std::move(callback)](const std::error_code &error, asio::ip::tcp::resolver::results_type endpoints)
                {
                    if (error)
                    {
                        cb(error);
                    }
                    else
                    {

                        asio::async_connect(
                            *socket,
                            endpoints,
                            [this, host = std::move(host), port, cb = std::move(cb)](const std::error_code &err, asio::ip::tcp::endpoint)
                            {
                                if (err)
                                {
                                    cb(err);
                                }
                                else
                                {
                                    asio::ip::tcp::no_delay option(true);
                                    socket->set_option(option);
                                    logger->info("connection made");
                                    connected = true;
                                    this->host = host;
                                    this->port = port;
                                    cb({});
                                }
                            });
                    }
                });
        }
        else
        {
            callback({});
        }
    }
    void insecure_http_session::reconnect(initialization_callback callback)
    {
        if (socket && socket->is_open())
        {
            socket->cancel();
            socket->close();
            logger->info("closed connection");
            connected = false;
            timer.expires_from_now(asio::chrono::milliseconds(500));
            timer.async_wait([this, cb = std::move(callback)](const std::error_code &error)
                             {
                                        if(error)
                                        {
                                            cb(error);
                                        } else 
                                        {
                                
                                            connect(host, port, std::move(cb));
                                        } });
        }
    }
    void insecure_http_session::shutdown()
    {
        if (socket)
        {
            if (socket->is_open())
            {
                socket->close();
            }
        }
    }
    bool insecure_http_session::is_scheme_matched(const std::string &scheme)
    {
        return this->scheme == scheme;
    }
    bool insecure_http_session::is_connected()
    {
        return connected;
    }
    void insecure_http_session::delay(execution_callback callback)
    {
        timer.expires_from_now(asio::chrono::milliseconds(500));
        timer.async_wait(
            [cb = std::move(callback)](const std::error_code &error)
            {
                if (error)
                {
                    cb(error);
                }
                else
                {

                    cb({});
                }
            });
    }
    void insecure_http_session::async_write(const std::vector<uint8_t> &out, transfer_callback &&callback)
    {
        asio::async_write(
            *socket,
            asio::buffer(out),
            callback);
    }
    void insecure_http_session::read(asio::streambuf &buffer, transfer_callback &&callback)
    {
        asio::async_read(*socket, buffer, callback);
    }
    void insecure_http_session::read_until(asio::streambuf &buffer, std::string_view delimiter, transfer_callback &&callback)
    {
        asio::async_read_until(*socket, buffer, delimiter, callback);
    }
    void insecure_http_session::read_exactly(asio::streambuf &buffer, std::size_t transfer_limit, transfer_callback &&callback)
    {
        asio::async_read(*socket, buffer, asio::transfer_exactly(transfer_limit), callback);
    }
    void insecure_http_session::read_header(asio::streambuf &buffer, transfer_callback &&callback)
    {
        asio::async_read_until(*socket, buffer, header_end_match(), callback);
    }
    void insecure_http_session::resolve_async(const std::string &host, uint16_t port, resolver_callback callback)
    {
        if (is_ip_address(host))
        {
            asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(host), port);
            resolver.async_resolve(endpoint, callback);
        }
        else if (port == 80)
        {
            resolver.async_resolve(host, scheme, callback);
        }
        else
        {
            resolver.async_resolve(host, fmt::format("{}", port), callback);
        }
    }
    bool insecure_http_session::is_ip_address(const std::string &ip)
    {
        struct sockaddr_in socket_address;
        if ((inet_pton(AF_INET, ip.c_str(), &(socket_address.sin_addr))))
        {
            return true;
        }
        return false;
    }
    insecure_http_session::~insecure_http_session()
    {
        if (socket)
        {
            if (socket->is_open())
            {
                socket->close();
            }
        }
    }
}