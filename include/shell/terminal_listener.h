#ifndef __REMOTE_SHELL_SHELL_TERMINAL_LISTENER__
#define __REMOTE_SHELL_SHELL_TERMINAL_LISTENER__

#include <cstdint>
#include <vector>
#include <system_error>

namespace shell
{
    class TerminalListener
    {
    public:
        virtual void on_terminal_initialized() = 0;
        virtual void on_terminal_data_received(const std::vector<uint8_t> &content) = 0;
        virtual void on_terminal_error(const std::error_code &err) = 0;
    };
}
#endif //__REMOTE_SHELL_SHELL_TERMINAL_LISTENER__