#ifndef __DAEMON_DOMAIN_CONTAINERS_RUNTIME_LISTENER__
#define __DAEMON_DOMAIN_CONTAINERS_RUNTIME_LISTENER__

#include <string>
#include <system_error>

namespace domain::containers
{
    class runtime_listener
    {
    public:
        virtual void container_initialized(const std::string &identifier) = 0;
        virtual void container_started(const std::string &identifier) = 0;
        virtual void container_failed(const std::string &identifier, std::error_code error) = 0;
        virtual void container_stopped(const std::string &identifier) = 0;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_RUNTIME_LISTENER__