#ifndef __DAEMON_DOMAIN_CONTAINERS_VIRTUAL_TERMINAL__
#define __DAEMON_DOMAIN_CONTAINERS_VIRTUAL_TERMINAL__

#include <system_error>
#include <cstdint>
#include <vector>

namespace domain::containers
{
    class virtual_terminal
    {
    public:
        virtual ~virtual_terminal() = default;
        virtual std::error_code initialize() = 0;
        virtual void start() = 0;
        virtual void resize(uint32_t columns, uint32_t rows) = 0;
        virtual void write(const std::vector<uint8_t> &content) = 0;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_VIRTUAL_TERMINAL__