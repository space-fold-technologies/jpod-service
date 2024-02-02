#ifndef __DAEMON_DOMAIN_CONTAINERS_CONTAINER_LISTENER__
#define __DAEMON_DOMAIN_CONTAINERS_CONTAINER_LISTENER__

#include <vector>
#include <string>
#include <system_error>

namespace domain::containers
{
    enum class listener_category
    {
        runtime,
        observer
    };
    class container_listener
    {
    public:
        virtual void on_operation_initialization() = 0;
        virtual void on_operation_output(const std::vector<uint8_t> &content) = 0;
        virtual void on_operation_failure(const std::error_code &error) = 0;
        virtual listener_category type() = 0;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_CONTAINER_LISTENER__