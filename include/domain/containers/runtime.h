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
    class container_repository;
    using monitor_provider = std::function<std::shared_ptr<container_monitor>()>;
    using address_assigner = std::function<std::error_code(const std::string &identifier, const std::string &network)>;
    using address_cleaner = std::function<std::error_code(const std::string &identifier, const std::string &network)>;
    class runtime : public runtime_listener
    {
    public:
        runtime(
            asio::io_context &context,
            std::shared_ptr<container_repository> repository,
            monitor_provider container_monitor_provider,
            address_assigner container_address_assigner,
            address_cleaner container_address_cleaner);
        virtual ~runtime();
        void create_container(operation_details details);
        void remove_container(const std::string &identifier);
        std::shared_ptr<container> fetch_container(const std::string &identifier);
        void container_initialized(const std::string &identifier, const std::string &network) override;
        void container_started(const std::string &identifier) override;
        void container_failed(const std::string &identifier, const std::error_code &error) override;
        void container_stopped(const std::string &identifier, const std::string& network) override;

    private:
        asio::io_context &context;
        std::shared_ptr<container_repository> repository;
        monitor_provider container_monitor_provider;
        address_assigner container_address_assigner;
        address_cleaner container_address_cleaner;
        std::map<std::string, std::shared_ptr<container>> containers;
        std::map<std::string, std::shared_ptr<container_monitor>> monitors;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_RUNTIME__