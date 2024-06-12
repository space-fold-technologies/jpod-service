#ifndef __DAEMON_CORE_HTTP_INSECURE_HTTP_SESSION__
#define __DAEMON_CORE_HTTP_INSECURE_HTTP_SESSION__

#include <core/http/session.h>
#include <memory>
#include <asio/steady_timer.hpp>
#include <optional>

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
    class insecure_http_session : public http_session, std::enable_shared_from_this<insecure_http_session>
    {
    public:
        explicit insecure_http_session(asio::io_context &context);
        virtual ~insecure_http_session();
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
        asio::steady_timer timer;
        std::optional<asio::ip::tcp::socket> socket;
        std::string scheme;
        std::string host;
        uint16_t port;
        bool connected;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_CORE_HTTP_INSECURE_HTTP_SESSION__