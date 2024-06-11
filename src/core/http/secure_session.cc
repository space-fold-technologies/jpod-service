#include <core/http/secure_session.h>
#include <core/http/utility.h>
#include <asio/io_context.hpp>
#include <asio/connect.hpp>
#include <asio/write.hpp>
#include <asio/read.hpp>
#include <arpa/inet.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
namespace core::http
{
    secure_http_session::secure_http_session(asio::io_context &context, asio::ssl::context &ssl_ctx) : resolver(context),
                                                                                                       ssl_ctx(ssl_ctx),
                                                                                                       timer(context),
                                                                                                       host(""),
                                                                                                       port(443),
                                                                                                       verified(false),
                                                                                                       is_under_lock(false),
                                                                                                       scheme("https"),
                                                                                                       logger(spdlog::get("jpod"))
    {
        stream.emplace(context, ssl_ctx);
        // consider making the secure connection inherit `std::enable_shared_from_this<T>` and have each `lambda` take it in
    }

    void secure_http_session::connect(const std::string &host, uint16_t port, initialization_callback callback)
    {
        if (!verified)
        {
            is_under_lock = true;
            resolve_async(
                host,
                port,
                [this, host, port, cb = std::move(callback), _(shared_from_this())](const std::error_code &error, asio::ip::tcp::resolver::results_type endpoints)
                {
                    if (error)
                    {
                        logger->error("endpoint-resolve-failure:{}", error.message());
                        cb(error);
                    }
                    else
                    {
                        asio::async_connect(
                            stream->lowest_layer(),
                            endpoints,
                            [this, host, port, cb = std::move(cb), _(shared_from_this())](const std::error_code &err, asio::ip::tcp::endpoint)
                            {
                                if (err)
                                {
                                    cb(err);
                                }
                                else
                                {
                                    ssl_ctx.set_verify_callback(asio::ssl::host_name_verification(host));
                                    asio::ip::tcp::no_delay option(true);
                                    stream->lowest_layer().set_option(option);
                                    SSL_set_tlsext_max_fragment_length(stream->native_handle(), TLSEXT_max_fragment_length_4096);
                                    if (!SSL_set_tlsext_host_name(stream->native_handle(), host.c_str()))
                                    {
                                        asio::system_error ssl_error(std::error_code(static_cast<int>(::ERR_get_error()), asio::error::get_ssl_category()));
                                        logger->warn("Failed to set SNI host name: {} for SSL: {}", host, ssl_error.what());
                                        auto system_error = ssl_error.code();
                                        if (!system_error)
                                        {
                                            system_error = std::make_error_code(std::errc::not_supported);
                                        }
                                        cb(system_error);
                                    }
                                    else
                                    {
                                        SSL_clear(stream->native_handle());
                                        stream->async_handshake(
                                            asio::ssl::stream_base::client,
                                            [this, host, port, cb = std::move(cb), _(shared_from_this())](const std::error_code &hand_shake_error)
                                            {
                                                if (hand_shake_error)
                                                {
                                                    cb(hand_shake_error);
                                                }
                                                else
                                                {
                                                    logger->info("handshake successful for: {}", host);
                                                    this->host = host;
                                                    this->port = port;
                                                    this->verified = true;
                                                    cb({});
                                                }
                                            });
                                    }
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
    void secure_http_session::reconnect(initialization_callback callback)
    {
        stream->lowest_layer().cancel();
        stream->async_shutdown([this, cb = std::move(callback), _(shared_from_this())](const std::error_code &error)
                               {
                                if(error)
                                {
                                    logger->error("reconnection ssl close error: {}", error.message());
                                } 
                                else
                                {
                                    stream->lowest_layer().close();
                                    logger->info("closed old secure connection");  
                                    verified = false;
                                    timer.expires_from_now(asio::chrono::milliseconds(500));  
                                    timer.async_wait([this,cb=std::move(cb), _(shared_from_this())](const std::error_code& error){
                                        if(error)
                                        {
                                            cb(error);
                                        } else 
                                        {
                                            SSL_clear(stream->native_handle());
                                            stream.emplace(stream->get_executor(), ssl_ctx);
                                            connect(host, port, std::move(cb));
                                        }
                                    });
                                } });
        // how to make a convincing reconnection operation
    }
    void secure_http_session::shutdown()
    {
        if (stream)
        {
            std::error_code error{};
            if (stream->lowest_layer().cancel(error); error)
            {
                logger->error("failed to cancel socket: {}", error.message());
            }
            if (stream->shutdown(error); error)
            {
                logger->error("shutdown ssl close error: {}", error.message());
            }
            if(stream->lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, error); !error)
            {
                stream->lowest_layer().close();
            }

            SSL_clear(stream->native_handle());
            stream.reset();
            logger->info("dumping secure connection");
        }
    }
    bool secure_http_session::is_scheme_matched(const std::string &scheme)
    {
        return this->scheme == scheme;
    }
    bool secure_http_session::is_connected()
    {
        return verified;
    }
    void secure_http_session::delay(execution_callback callback)
    {
        timer.expires_after(asio::chrono::seconds(5));
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
    void secure_http_session::async_write(const std::vector<uint8_t> &out, transfer_callback &&callback)
    {
        asio::async_write(
            *stream,
            asio::buffer(out),
            callback);
    }
    void secure_http_session::read(asio::streambuf &buffer, transfer_callback &&callback)
    {
        stream->lowest_layer().async_wait(
            asio::ip::tcp::socket::wait_read,
            [this, &buffer, &callback, _(shared_from_this())](const std::error_code &error)
            {
                asio::async_read(*stream, buffer, callback);
            });
    }
    void secure_http_session::read_until(asio::streambuf &buffer, std::string_view delimiter, transfer_callback &&callback)
    {
        asio::async_read_until(*stream, buffer, delimiter, callback);
    }
    void secure_http_session::read_exactly(asio::streambuf &buffer, std::size_t transfer_limit, transfer_callback &&callback)
    {
        asio::async_read(*stream, buffer, asio::transfer_exactly(transfer_limit), callback);
    }
    void secure_http_session::read_header(asio::streambuf &buffer, transfer_callback &&callback)
    {
        asio::async_read_until(*stream, buffer, header_end_match(), callback);
    }
    void secure_http_session::resolve_async(const std::string &host, uint16_t port, resolver_callback callback)
    {
        if (is_ip_address(host))
        {
            asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(host), port);
            resolver.async_resolve(endpoint, callback);
        }
        else if (port == 443)
        {
            resolver.async_resolve(host, scheme, callback);
        }
        else
        {
            resolver.async_resolve(host, fmt::format("{}", port), callback);
        }
    }
    bool secure_http_session::is_ip_address(const std::string &ip)
    {
        struct sockaddr_in socket_address;
        if ((inet_pton(AF_INET, ip.c_str(), &(socket_address.sin_addr))))
        {
            return true;
        }
        return false;
    }
    /*

        if ((error.category() == asio::error::get_ssl_category()) && (SSL_R_PROTOCOL_IS_SHUTDOWN == ERR_GET_REASON(error.value())))
                    {
                        stream->lowest_layer().close();
                    }
    */
    secure_http_session::~secure_http_session()
    {
        if (stream)
        {
            std::error_code error{};
            if (stream->lowest_layer().cancel(error); error)
            {
                logger->error("failed to cancel socket: {}", error.message());
            }
            if (stream->shutdown(error); error)
            {
                logger->error("shutdown ssl close error: {}", error.message());
            }
            if(stream->lowest_layer().shutdown(asio::ip::tcp::socket::shutdown_both, error); !error)
            {
                stream->lowest_layer().close();
            }

            SSL_clear(stream->native_handle());
            stream.reset();
            logger->info("dumping secure connection");
        }
    }
}