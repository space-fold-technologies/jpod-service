#ifndef __DAEMON_DOMAIN_NETWORKING_REMOVE_HANDLER__
#define __DAEMON_DOMAIN_NETWORKING_REMOVE_HANDLER__
#include <core/commands/command_handler.h>
#include <memory>

namespace spdlog
{
    class logger;
};
namespace domain::networking
{
    class network_service;
    class remove_handler : public core::commands::command_handler
    {
    public:
        remove_handler(
            core::connections::connection &connection,
            std::shared_ptr<network_service> service);
        virtual ~remove_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;

    private:
        std::shared_ptr<network_service> service;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif //__DAEMON_DOMAIN_NETWORKING_REMOVE_HANDLER__