#ifndef __DAEMON_DOMAIN_CONTAINERS_REMOVAL_COMMAND_HANDLER__
#define __DAEMON_DOMAIN_CONTAINERS_REMOVAL_COMMAND_HANDLER__

#include <core/commands/command_handler.h>
#include <system_error>
#include <memory>

namespace spdlog
{
    class logger;
};

namespace domain::containers
{
    class container_repository;
    class container_removal_handler : public core::commands::command_handler
    {
    public:
        container_removal_handler(
            core::connections::connection &connection,
            std::shared_ptr<container_repository> repository);
        virtual ~container_removal_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;

    private:
        std::shared_ptr<container_repository> repository;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_REMOVAL_COMMAND_HANDLER__