#ifndef __DAEMON_CORE_HTTP_HTTP_SESSION__
#define __DAEMON_CORE_HTTP_HTTP_SESSION__

#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <asio/posix/stream_descriptor.hpp>
#include <string_view>

namespace core::http
{
    using initialization_callback = std::function<void(const std::error_code &)>;
    using resolver_callback = std::function<void(const std::error_code &, asio::ip::tcp::resolver::results_type)>;
    using transfer_callback = std::function<void(const std::error_code &, std::size_t)>;
    using execution_callback = std::function<void(const std::error_code &)>;
    
    class http_session
    {
    public:
        virtual ~http_session() = default;
        virtual void connect(const std::string &host, uint16_t port, initialization_callback callback) = 0;
        virtual void reconnect(initialization_callback callback) = 0;
        virtual void shutdown() = 0;
        virtual bool is_scheme_matched(const std::string &scheme) = 0;
        virtual bool is_connected() = 0;
        virtual void delay(execution_callback callback) = 0;
        virtual void async_write(const std::vector<uint8_t> &out, transfer_callback &&callback) = 0;
        virtual void read(asio::streambuf &buffer, transfer_callback &&callback) = 0;
        virtual void read_until(asio::streambuf &buffer, std::string_view delimiter, transfer_callback &&callback) = 0;
        virtual void read_exactly(asio::streambuf &buffer, std::size_t transfer_limit, transfer_callback &&callback) = 0;
        virtual void read_header(asio::streambuf &buffer, transfer_callback &&callback) = 0;
    };
}

#endif //__DAEMON_CORE_HTTP_HTTP_SESSION__