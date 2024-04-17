#ifndef __DAEMON_CORE_CONNECTIONS_CONNECTION__
#define __DAEMON_CORE_CONNECTIONS_CONNECTION__

#include <asio/local/stream_protocol.hpp>
#include <asio/streambuf.hpp>
#include <asio/strand.hpp>
#include <asio/steady_timer.hpp>
#include <vector>
#include <deque>
#include <cstdint>
#include <atomic>
#include <memory>
#include <mutex>
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
    static constexpr uint8_t _header_operation_mask = 0b00011111;
    static constexpr uint8_t _header_target_mask = 0b11100000;
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
        void read_header();
        void read_payload(uint8_t target, uint8_t operation, std::size_t payload_length);
        void read_medium_payload_length(uint8_t target, uint8_t operation);
        void read_large_payload_length(uint8_t target, uint8_t operation);
        void write(uint8_t target, uint8_t operation, const std::vector<uint8_t> &payload);
        void on_error(const std::error_code &error);
        void send_from_queue();
        void cancel_timeout();
        void initiate_timeout();
        void handle_payload(uint8_t target, uint8_t operation, const std::vector<uint8_t> &payload);

    private:
        std::string identifier;
        asio::local::stream_protocol::socket socket;
        asio::streambuf buffer;
        std::deque<std::vector<uint8_t>> sending_queue;
        std::mutex lock;
        std::unique_ptr<asio::steady_timer> timer;
        std::shared_ptr<core::commands::command_handler_registry> command_handler_registry;
        removal_trigger_callback removal_callback;
        std::shared_ptr<core::commands::command_handler> command_handler;
        std::string command_handler_key;
        std::shared_ptr<spdlog::logger> logger;
    };
};

#endif // __DAEMON_CORE_CONNECTIONS_CONNECTION__