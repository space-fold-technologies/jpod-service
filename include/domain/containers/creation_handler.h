#ifndef __DAEMON_DOMAIN_CONTAINERS_CREATION_HANDLER__
#define __DAEMON_DOMAIN_CONTAINERS_CREATION_HANDLER__

#include <core/commands/command_handler.h>
#include <tl/expected.hpp>
#include <system_error>
#include <filesystem>
#include <memory>
#include <map>

namespace spdlog
{
    class logger;
};

struct archive;
using archive_ptr = std::shared_ptr<archive>;
namespace fs = std::filesystem;
namespace domain::containers
{
    class container_repository;
    struct creation_state
    {
        fs::path image_folder;
        fs::path container_folder;
        std::string image_identifier;
        std::string container_identifier;
        std::string name;
        std::string network_properties;
        std::map<std::string, std::string> port_map;
        std::map<std::string, std::string> env_vars;
        std::map<std::string, std::string> volumes;
        std::shared_ptr<container_repository> store;
    };
    using creation_result = tl::expected<creation_state, std::error_code>;
    class creation_handler : public core::commands::command_handler
    {

    public:
        creation_handler(core::connections::connection &connection,
                         const fs::path &containers_folder,
                         const fs::path &images_folder,
                         std::shared_ptr<container_repository> repository);
        virtual ~creation_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;

    private:
        static creation_result initialize_creation(
            std::string identifier,
            std::shared_ptr<container_repository> store,
            const fs::path &images_folder,
            const fs::path &containers_folder,
            const std::vector<uint8_t> &payload);
        static creation_result extract_layers(creation_state state);
        static tl::expected<std::string, std::error_code> register_container(creation_state state);

    private:
        const fs::path &containers_folder;
        const fs::path &images_folder;
        std::shared_ptr<container_repository> repository;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_CREATION_HANDLER__