#ifndef __DAEMON_CORE_COMMANDS_COMMAND_HANDLER__
#define __DAEMON_CORE_COMMANDS_COMMAND_HANDLER__

#include <string>
#include <cstdint>
#include <vector>
#include <system_error>
#include <deque>
#include <array>
#include <memory>

namespace spdlog
{
    class logger;
};
namespace core::connections
{
    class connection;
};
namespace core::commands
{
    class command_handler
    {
        friend core::connections::connection;

    protected:
        command_handler(core::connections::connection &connection);
        void send_error(const std::error_code &err);
        void send_error(const std::string &err);
        void send_frame(const std::vector<uint8_t> &payload);
        void send_success(const std::string& message);
        void send_progress(const std::vector<uint8_t>& data);
        void send_close(const std::vector<uint8_t> &payload);

    public:
        virtual ~command_handler() = default;
        virtual void on_order_received(const std::vector<uint8_t> &payload) = 0;
        virtual void on_connection_closed(const std::error_code &error) = 0;

    private:
        core::connections::connection &connection;
        std::deque<std::vector<uint8_t>> sending_queue;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_CORE_COMMANDS_COMMAND_HANDLER__