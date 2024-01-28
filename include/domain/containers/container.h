#ifndef __DAEMON_DOMAIN_CONTAINERS_CONTAINER__
#define __DAEMON_DOMAIN_CONTAINERS_CONTAINER__

#include <domain/containers/orders.h>

namespace domain::containers
{
    class container
    {
    public:
        virtual void initialize(const container_details &details) = 0;
        virtual ~container() = default;
    };
}

#endif // __DAEMON_DOMAIN_CONTAINERS_CONTAINER__