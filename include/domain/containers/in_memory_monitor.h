#ifndef __DAEMON_DOMAIN_CONTAINERS_IN_MEMORY_MONITOR__
#define __DAEMON_DOMAIN_CONTAINERS_IN_MEMORY_MONITOR__

#include <domain/containers/monitor.h>
#include <memory>

namespace spdlog
{
    class logger;
};

namespace domain::containers
{
    class in_memory_monitor : public container_monitor
    {
    public:
        in_memory_monitor();
        virtual ~in_memory_monitor();
        void on_operation_initialization() override;
        void on_operation_output(const std::vector<uint8_t> &content) override;
        void on_operation_failure(const std::error_code &error) override;
        listener_category type() override;
        void clear() override;

    private:
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_CONTAINERS_IN_MEMORY_MONITOR__