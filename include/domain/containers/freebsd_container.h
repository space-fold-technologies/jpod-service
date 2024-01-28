#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER__

#include <domain/containers/container.h>
namespace domain::containers
{

    class freebsd_container : public container
    {
    public:
        freebsd_container();
        virtual ~freebsd_container();
        void initialize(const container_details &details) override;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER__