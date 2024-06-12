#ifndef __DAEMON_CORE_HTTP_SECURE_HTTP_SESSION__
#define __DAEMON_CORE_HTTP_SECURE_HTTP_SESSION__

#include <core/http/session.h>
#include <asio/ssl.hpp>
#include <asio/steady_timer.hpp>
#include <memory>
#include <optional>
#include <atomic>

namespace asio
{
    class io_context;
};

namespace spdlog
{
    class logger;
};

namespace core::http
{
    class secure_http_session : public http_session, public std::enable_shared_from_this<secure_http_session>
    {
    public:
        explicit secure_http_session(asio::io_context &context, asio::ssl::context &ssl_ctx);
        virtual ~secure_http_session();
        void connect(const std::string &host, uint16_t port, initialization_callback callback) override;
        void reconnect(initialization_callback callback) override;
        void shutdown() override;
        bool is_scheme_matched(const std::string &scheme) override;
        bool is_connected() override;
        void delay(execution_callback callback) override;
        void async_write(const std::vector<uint8_t> &out, transfer_callback &&callback) override;
        void read(asio::streambuf &buffer, transfer_callback &&callback) override;
        void read_until(asio::streambuf &buffer, std::string_view delimiter, transfer_callback &&callback) override;
        void read_exactly(asio::streambuf &buffer, std::size_t transfer_limit, transfer_callback &&callback) override;
        void read_header(asio::streambuf &buffer, transfer_callback &&callback) override;

    private:
        void resolve_async(const std::string &host, uint16_t port, resolver_callback callback);
        bool is_ip_address(const std::string &ip);

    private:
        asio::ip::tcp::resolver resolver;
        asio::ssl::context &ssl_ctx;
        asio::steady_timer timer;
        std::string host;
        uint16_t port;
        bool verified;
        std::atomic_bool is_under_lock;
        std::optional<asio::ssl::stream<asio::ip::tcp::socket>> stream;
        std::string scheme;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_CORE_HTTP_SECURE_HTTP_SESSION__