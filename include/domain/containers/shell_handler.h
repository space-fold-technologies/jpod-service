#ifndef __DAEMON_DOMAIN_CONTAINERS_SHELL_HANDLER__
#define __DAEMON_DOMAIN_CONTAINERS_SHELL_HANDLER__

#include <core/commands/command_handler.h>
#include <domain/containers/terminal_listener.h>
#include <functional>
#include <memory>

namespace spdlog
{
    class logger;
};

namespace domain::containers
{
    class container_repository;
    class virtual_terminal;

    typedef std::function<std::unique_ptr<virtual_terminal>(const std::string &, terminal_listener &)> virtual_terminal_provider;
    class shell_handler : public core::commands::command_handler, public terminal_listener
    {
    public:
        explicit shell_handler(core::connections::connection &connection,
                      std::shared_ptr<container_repository> repository,
                      virtual_terminal_provider provider);
        virtual ~shell_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;
        void on_terminal_initialized() override;
        void on_terminal_data_received(const std::vector<uint8_t> &content) override;
        void on_terminal_error(const std::error_code &error) override;

    private:
        std::shared_ptr<container_repository> repository;
        virtual_terminal_provider provider;
        std::unique_ptr<virtual_terminal> terminal;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_CONTAINERS_SHELL_HANDLER__