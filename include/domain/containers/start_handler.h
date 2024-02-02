#ifndef __DAEMON_DOMAIN_CONTAINERS_START_HANDLER__
#define __DAEMON_DOMAIN_CONTAINERS_START_HANDLER__

#include <core/commands/command_handler.h>
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
    class start_handler : public core::commands::command_handler
    {
    public:
        start_handler(
            core::connections::connection &connection,
            std::shared_ptr<container_repository> repository,
            std::shared_ptr<runtime> runtime_ptr,
            const fs::path &containers_folder);
        virtual ~start_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;

    private:
        
    private:
        std::shared_ptr<container_repository> repository;
        std::shared_ptr<runtime> runtime_ptr;
        const fs::path &containers_folder;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_START_HANDLER__