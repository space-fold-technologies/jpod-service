#ifndef __DAEMON_CORE_CONNECTIONS_CONNECTION__
#define __DAEMON_CORE_CONNECTIONS_CONNECTION__

#include <asio/local/stream_protocol.hpp>
#include <asio/streambuf.hpp>
#include <asio/strand.hpp>
#include <vector>
#include <deque>
#include <cstdint>
#include <functional>

#if (defined(ASIO_STANDALONE) && ASIO_VERSION >= 101800)
#include <asio/any_io_executor.hpp>
using strand = asio::strand<asio::any_io_executor>;
#else
#include <asio/executor.hpp>
using strand = asio::strand<asio::executor>;
#endif

namespace spdlog
{
    class logger;
};
namespace core::commands
{
    class command_handler;
    class command_handler_registry;
};
namespace core::connections
{
    class connection
    {
        friend core::commands::command_handler;
        typedef std::function<void(const std::string &identifier)> removal_trigger_callback;

    public:
        explicit connection(
            std::string identifier,
            asio::local::stream_protocol::socket socket,
            std::shared_ptr<core::commands::command_handler_registry> command_handler_registry,
            removal_trigger_callback removal_callback);
        virtual ~connection();
        void start();

    private:
        void read_payload();
        void write(std::vector<uint8_t> &payload);
        void on_error(const std::error_code &error);
        void send_from_queue();

    private:
        std::string identifier;
        asio::local::stream_protocol::socket socket;
        // asio::streambuf stream_buffer;
        std::vector<uint8_t> buffer;
        std::deque<std::vector<uint8_t>> sending_queue;
        std::shared_ptr<core::commands::command_handler_registry> command_handler_registry;
        removal_trigger_callback removal_callback;
        std::shared_ptr<core::commands::command_handler> command_handler;
        std::shared_ptr<spdlog::logger> logger;
    };
};

#endif // __DAEMON_CORE_CONNECTIONS_CONNECTION__