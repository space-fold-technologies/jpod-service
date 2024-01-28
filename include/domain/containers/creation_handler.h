#ifndef __DAEMON_DOMAIN_CONTAINERS_CREATION_HANDLER__
#define __DAEMON_DOMAIN_CONTAINERS_CREATION_HANDLER__

#include <core/commands/command_handler.h>

namespace domain::containers
{
    class runtime;
    class creation_handler : public core::commands::command_handler
    {
    public:
        creation_handler(core::connections::connection &connection, std::shared_ptr<runtime> runtime_ptr);
        virtual ~creation_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;

    private:
        std::shared_ptr<runtime> runtime_ptr;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_CREATION_HANDLER__