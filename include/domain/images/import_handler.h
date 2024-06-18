#ifndef __DAEMON_DOMAIN_IMAGES_IMPORT_COMMAND_HANDLER__
#define __DAEMON_DOMAIN_IMAGES_IMPORT_COMMAND_HANDLER__

#include <core/commands/command_handler.h>
#include <system_error>
#include <filesystem>
#include <memory>
#include <map>
#include <vector>
#include <tl/expected.hpp>

namespace spdlog
{
    class logger;
};
struct archive;
namespace fs = std::filesystem;
namespace domain::images
{
    class image_repository;
    struct import_order;
    using archive_ptr = std::shared_ptr<archive>;
    constexpr std::size_t BUFFER_SIZE = 10240;
    const std::size_t WRITE_BUFFER_SIZE = 1024 * 100;
    const std::string MANIFEST = "manifest.json";
    const std::string INDEX = "index.json";

    struct import_state
    {
        std::vector<uint8_t> manifest;
        std::vector<uint8_t> index;
        std::string identifier;
        std::string registry;
        std::string repository;
        std::string tag;
        std::size_t size;
        std::string os;
        std::string variant;
        std::string version;
        fs::path image_folder;
        fs::path image_archive;
        std::map<std::string, fs::path> entries;
        std::shared_ptr<image_repository> store;
        std::map<std::string, std::string> env_vars;
        std::map<uint16_t, std::string> exposed_ports;
        std::map<std::string, std::string> labels;
        std::vector<std::string> volumes;
        std::vector<std::string> command;
        std::vector<std::string> entry_point;
        std::shared_ptr<spdlog::logger> logger;
    };

    using import_result = tl::expected<import_state, std::error_code>;

    class import_handler : public core::commands::command_handler
    {
    public:
        import_handler(
            core::connections::connection &connection,
            fs::path &image_folder,
            std::shared_ptr<image_repository> repository);
        virtual ~import_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;

    private:
        static import_result initialize_state(std::shared_ptr<image_repository> repository, fs::path local_file_path, fs::path &image_folder);
        static import_result read_image_details(import_state state);
        static import_result extract_configuration(import_state state);
        static import_result extract_index(import_state state);
        static import_result extract_image_content(import_state state);
        static import_result resolve_image_meta(import_state state);
        static tl::expected<std::string, std::error_code> persist_image_details(import_state state);

    private:
        std::shared_ptr<image_repository> repository;
        fs::path &image_folder;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_IMPORT_COMMAND_HANDLER__
