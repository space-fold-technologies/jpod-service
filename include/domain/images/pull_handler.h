#ifndef __DAEMON_DOMAIN_IMAGES_PULL_COMMAND_HANDLER__
#define __DAEMON_DOMAIN_IMAGES_PULL_COMMAND_HANDLER__

#include <core/commands/command_handler.h>
#include <functional>
#include <filesystem>
#include <memory>
#include <map>

namespace spdlog
{
    class logger;
};

namespace core::oci
{
    class oci_client;
    struct image_properties;
    struct progress_update;
};
namespace fs = std::filesystem;
using namespace core::oci;
namespace domain::images
{
    using oci_client_provider = std::function<std::unique_ptr<core::oci::oci_client>()>;
    class image_repository;
    class registry_access_details;
    struct progress_frame;
    class pull_handler : public core::commands::command_handler
    {
    public:
        explicit pull_handler(
            core::connections::connection &connection,
            std::shared_ptr<image_repository> repository,
            oci_client_provider provider,
            const fs::path &image_folder);
        virtual ~pull_handler();
        void on_order_received(const std::vector<uint8_t> &payload) override;
        void on_connection_closed(const std::error_code &error) override;

    private:
        void fetch_oci_image(const registry_access_details &details, const std::string &repository, const std::string &tag);
        void on_image_download(const std::error_code &error, const progress_update &update, const image_properties &properties);
        std::error_code save_image_details(const std::string identifier, const image_properties &properties);

    private:
        std::shared_ptr<image_repository> repository;
        oci_client_provider provider;
        const fs::path &image_folder;
        std::unique_ptr<core::oci::oci_client> client;
        std::unique_ptr<progress_frame> frame;
        std::map<std::string, uint16_t> layer_progress;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_PULL_COMMAND_HANDLER__