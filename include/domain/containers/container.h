#ifndef __DAEMON_DOMAIN_CONTAINERS_CONTAINER__
#define __DAEMON_DOMAIN_CONTAINERS_CONTAINER__

#include <domain/containers/details.h>
#include <memory>

namespace domain::containers
{
    class container_listener;
    class container
    {

    public:
        container(container_details details) : details(std::move(details))
        {
        }
        virtual void initialize() = 0;
        virtual ~container() = default;
        virtual void start() = 0;
        virtual void resize(int columns, int rows) = 0;
        virtual void register_listener(std::shared_ptr<container_listener> listener) = 0;

    protected:
        container_details details;
    };
}

#endif // __DAEMON_DOMAIN_CONTAINERS_CONTAINER__