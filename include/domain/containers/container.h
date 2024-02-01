#ifndef __DAEMON_DOMAIN_CONTAINERS_CONTAINER__
#define __DAEMON_DOMAIN_CONTAINERS_CONTAINER__

#include <domain/containers/details.h>
#include <domain/containers/runtime_listener.h>
#include <memory>

namespace domain::containers
{
    class container_listener;
    class container
    {

    public:
        container(container_details details, runtime_listener &listener) : details(std::move(details)), listener(listener)
        {
        }
        virtual void initialize() = 0;
        virtual ~container() = default;
        virtual void start() = 0;
        virtual void resize(int columns, int rows) = 0;
        virtual void register_listener(std::shared_ptr<container_listener> operation_listener) = 0;

    protected:
        container_details details;
        runtime_listener &listener;
    };
}

#endif // __DAEMON_DOMAIN_CONTAINERS_CONTAINER__