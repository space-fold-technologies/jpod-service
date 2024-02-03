#ifndef __DAEMON_DOMAIN_CONTAINERS_RUNTIME__
#define __DAEMON_DOMAIN_CONTAINERS_RUNTIME__

#include <domain/containers/runtime_listener.h>
#include <domain/containers/container.h>
#include <functional>
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
    class container_monitor;
    typedef std::function<std::shared_ptr<container_monitor>()> monitor_provider;
    class runtime : public runtime_listener
    {
    public:
        runtime(asio::io_context &context, monitor_provider container_monitor_provider);
        virtual ~runtime();
        void create_container(operation_details details);
        void remove_container(std::string &identifier);
        std::shared_ptr<container> fetch_container(const std::string &identifier);
        void container_initialized(const std::string &identifier) override;
        void container_started(const std::string &identifier) override;
        void container_failed(const std::string &identifier, const std::error_code &error) override;
        void container_stopped(const std::string &identifier) override;

    private:
        asio::io_context &context;
        monitor_provider container_monitor_provider;
        std::map<std::string, std::shared_ptr<container>> containers;
        std::map<std::string, std::shared_ptr<container_monitor>> monitors;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_RUNTIME__