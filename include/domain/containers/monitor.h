#ifndef __DAEMON_DOMAIN_CONTAINERS_MONITOR__
#define __DAEMON_DOMAIN_CONTAINERS_MONITOR__

#include <domain/containers/container_listener.h>

namespace domain::containers
{
    class container_monitor : public container_listener
    {
    public:
        virtual void clear() = 0;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_MONITOR__