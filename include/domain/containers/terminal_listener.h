#ifndef __DAEMON_DOMAIN_CONTAINERS_TERMINAL_LISTENER__
#define __DAEMON_DOMAIN_CONTAINERS_TERMINAL_LISTENER__

#include <cstdint>
#include <vector>
#include <system_error>

namespace domain::containers
{
    class terminal_listener
    {
    public:
        virtual void on_terminal_data_received(const std::vector<uint8_t> &content) = 0;
        virtual void on_terminal_error(const std::error_code &err) = 0;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_TERMINAL_LISTENER__