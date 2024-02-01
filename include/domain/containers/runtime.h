#ifndef __DAEMON_DOMAIN_CONTAINERS_RUNTIME__
#define __DAEMON_DOMAIN_CONTAINERS_RUNTIME__

#include <domain/containers/runtime_listener.h>
#include <domain/containers/container.h>
#include <memory>
#include <string>
#include <map>

namespace spdlog
{
    class logger;
};

namespace asio
{
    class io_context;
};

namespace domain::containers
{
    class container;
    class runtime: public runtime_listener
    {
    public:
        runtime(asio::io_context &context);
        virtual ~runtime();
        void create_container(operation_details details);
        void remove_container(std::string &identifier);
        void container_initialized(const std::string &identifier) override;
        void container_started(const std::string &identifier) override;
        void container_failed(const std::string &identifier, const std::error_code &error) override;
        void container_stopped(const std::string &identifier) override;

    private:
        asio::io_context &context;
        std::map<std::string, std::shared_ptr<container>> containers;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_RUNTIME__