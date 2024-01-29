#ifndef __DAEMON_DOMAIN_CONTAINERS_CONTAINER__
#define __DAEMON_DOMAIN_CONTAINERS_CONTAINER__

#include <domain/containers/orders.h>
#include <memory>
namespace domain::containers
{
    class container_listener;
    class container
    {
    public:
        virtual void initialize() = 0;
        virtual ~container() = default;
        virtual void start() = 0;
        void resize(int columns, int rows);
        virtual void register_listener(std::shared_ptr<container_listener> listener) = 0;
    };
}

#endif // __DAEMON_DOMAIN_CONTAINERS_CONTAINER__