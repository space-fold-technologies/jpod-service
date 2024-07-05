#ifndef __DAEMON_DOMAIN_CONTAINERS_START_HANDLER__
#define __DAEMON_DOMAIN_CONTAINERS_START_HANDLER__

#include <core/commands/command_handler.h>
#include <domain/containers/container.h>
#include <tl/expected.hpp>
#include <system_error>
#include <filesystem>
#include <memory>
#include <vector>
#include <map>

namespace spdlog
{
    class logger;
};

namespace fs = std::filesystem;

namespace domain::images
{
    struct mount_point;
};

namespace domain::containers
{
    class container_repository;
    struct operation_details;
    class runtime;
    struct startup_state
    {
        std::string term;
        std::string user;
        std::string image_identifier;
        std::string os;
        std::string configuration;
        fs::path containers_folder;
        fs::path images_folder;
        std::map<std::string, std::string> env_vars;
        std::map<std::string, std::string> port_map;
        std::vector<std::string> entry_point;
        std::vector<std::string> command;
        std::shared_ptr<runtime> runtime_ptr;
        std::shared_ptr<container_repository> store;
        operation_details details;
    };
    using startup_result = tl::expected<startup_state, std::error_code>;
    class start_handler : public core::commands::command_handler
    {
    public:
        explicit start_handler(
            core::connections::connection &connection,
            std::shared_ptr<container_repository> repository,
            std::shared_ptr<runtime> runtime_ptr,
            const fs::path &containers_folder,
            const fs::path &images_folder);
        virtual ~start_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;

    private:
        static startup_result initialize(const std::string &term,
                                         const std::string &user,
                                         const fs::path &containers_folder,
                                         const fs::path &images_folder,
                                         std::shared_ptr<container_repository> store,
                                         std::shared_ptr<runtime> runtime_ptr);
        static startup_result fetch_details(startup_state state);
        static startup_result prepare_container(startup_state state);
        static startup_result setup_command(startup_state state);
        static tl::expected<std::string, std::error_code> start_container(startup_state state);

    private:
        std::shared_ptr<container_repository> repository;
        std::shared_ptr<runtime> runtime_ptr;
        const fs::path &containers_folder;
        const fs::path &images_folder;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_START_HANDLER__