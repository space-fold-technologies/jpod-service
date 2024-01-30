#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER__

#include <domain/containers/container.h>
#include <memory>
#include <optional>

namespace asio
{
    class io_context;
}

namespace spdlog
{
    class logger;
}

namespace domain::containers
{
    class container_listener;
    struct container_details;
}

namespace domain::containers::freebsd
{

    class freebsd_container : public container
    {
        const int WRITE_BUFFER_SIZE = 1024;
        struct user_details;

    public:
        freebsd_container(asio::io_context &context, const container_details &details);
        virtual ~freebsd_container();
        void initialize() override;
        void register_listener(std::shared_ptr<container_listener> listener) override;

    private:
        asio::io_context &context;
        const container_details &details;
        std::shared_ptr<spdlog::logger> logger;
        std::shared_ptr<container_listener> listener;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER__