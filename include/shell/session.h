#ifndef __REMOTE_SHELL_SHELL_SESSION__
#define __REMOTE_SHELL_SHELL_SESSION__

#include <asio/ip/tcp.hpp>
#include <asio/io_context.hpp>
#include <core/connections/connection_listener.h>
#include <shell/terminal.h>
#include <shell/terminal_listener.h>
#include <deque>
#include <memory>

namespace spdlog
{
    class logger;
};

namespace core::connections
{
    class Connection;
};

namespace shell
{

    enum class MessageType
    {
        RESIZE_REQUEST = 0x0,
        TERMINAL_INPUT = 0x1,
        TERMINAL_OUTPUT = 0x2,
        SESSION_MESSAGE = 0x3,
        SESSION_REQUEST = 0x4
    };

    class Session : public core::connections::ConnectionListener, public TerminalListener, public std::enable_shared_from_this<Session>
    {
    public:
        Session(std::string id, asio::io_context &context, std::shared_ptr<core::connections::Connection> connection);
        ~Session() = default;
        void on_open() override;
        void on_close() override;
        void on_message(const std::vector<uint8_t> &payload) override;
        void on_error(const std::error_code &ec) override;
        void on_terminal_initialized() override;
        void on_terminal_data_received(const std::vector<uint8_t> &content) override;
        void on_terminal_error(const std::error_code &err) override;

    private:
        void start_session(const std::string &container_identifier);
        void send_warning(const std::string &message);
        void send_error(const std::string &message);
        void send_info(const std::string &message);
        void write_to_buffer(MessageType type, const std::vector<uint8_t> &buffer);

    private:
        std::string id;
        std::shared_ptr<core::connections::Connection> connection;
        Terminal terminal;
        std::shared_ptr<spdlog::logger> logger;
    };

}; // namespace shell

#endif // __REMOTE_SHELL_SHELL_SESSION__