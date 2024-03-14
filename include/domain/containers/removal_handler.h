#ifndef __DAEMON_DOMAIN_CONTAINERS_REMOVAL_COMMAND_HANDLER__
#define __DAEMON_DOMAIN_CONTAINERS_REMOVAL_COMMAND_HANDLER__

#include <core/commands/command_handler.h>
#include <system_error>
#include <filesystem>
#include <memory>

namespace spdlog
{
    class logger;
};

namespace fs = std::filesystem;

namespace domain::containers
{
    class container_repository;
    class runtime;
    class container_removal_handler : public core::commands::command_handler
    {
    public:
        container_removal_handler(
            core::connections::connection &connection,
            std::shared_ptr<runtime> runtime_ptr,
            std::shared_ptr<container_repository> repository,
            const fs::path &containers_folder);
        virtual ~container_removal_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;
    private:
        //utilities for removing the container
        std::error_code remove_folder(const std::string& identifier);
    private:
        std::shared_ptr<container_repository> repository;
        std::shared_ptr<runtime> runtime_ptr;
        const fs::path &containers_folder;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_REMOVAL_COMMAND_HANDLER__