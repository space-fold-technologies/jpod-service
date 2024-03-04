#ifndef __DAEMON_DOMAIN_CONTAINERS_CREATION_HANDLER__
#define __DAEMON_DOMAIN_CONTAINERS_CREATION_HANDLER__

#include <core/commands/command_handler.h>
#include <domain/images/payload.h>
#include <system_error>
#include <filesystem>
#include <memory>

namespace spdlog
{
    class logger;
};

struct archive;
using archive_ptr = std::unique_ptr<archive, std::function<void(archive *)>>;
namespace fs = std::filesystem;
namespace domain::containers
{
    struct creation_configuration
    {
        std::string containers_folder;
        std::string images_folder;
    };
    class container_repository;
    class creation_handler : public core::commands::command_handler
    {
        const double PROGRESSION_PRECISION = 0.5;
        const int BUFFER_SIZE = 4096;

    public:
        creation_handler(core::connections::connection &connection, const creation_configuration &configuration, std::shared_ptr<container_repository> repository);
        virtual ~creation_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;

    private:
        fs::path generate_container_folder(const std::string &identifier, std::error_code &error);
        fs::path fetch_image_archive(std::string &image_identifier, std::error_code &error);
        std::error_code initialize_decompression(std::string &image_identifier);
        std::error_code extract_filesystem();
        std::error_code copy_entry(struct archive *in, struct archive *out);

    private:
        std::string identifier;
        const creation_configuration &configuration;
        std::shared_ptr<container_repository> repository;
        fs::path container_directory;
        archive_ptr input;
        archive_ptr output;
        domain::images::progress_frame frame;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_CREATION_HANDLER__