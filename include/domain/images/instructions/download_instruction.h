#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DOWNLOAD_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DOWNLOAD_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <domain/images/payload.h>
#include <memory>
#include <string>
#include <optional>
#include <filesystem>
#include <system_error>
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
}

namespace domain::images
{
    class image_repository;
    class registry_access_details;
    class image_meta;
}

namespace fs = std::filesystem;
using namespace core::oci;
namespace domain::images::instructions
{
    constexpr std::size_t DOWNLOAD_BUFFER_SIZE = 1024 * 1000;
    class directory_resolver;
    class instruction_listener;
    using oci_client_provider = std::function<std::unique_ptr<core::oci::oci_client>()>;
    class download_instruction : public instruction
    {
    public:
        explicit download_instruction(
            const std::string &identifier,
            const std::string &order,
            oci_client_provider provider,
            image_repository &repository,
            directory_resolver &resolver,
            instruction_listener &listener);
        virtual ~download_instruction();
        void execute() override;

    private:
        void fetch_oci_image(const registry_access_details &details, const std::string &repository, const std::string &tag);
        void on_image_download(const std::error_code &error, const progress_update &update, const image_properties &properties);
        std::error_code save_image_details(const std::string identifier, const image_properties &properties);

    private:
        std::string identifier;
        std::string order;
        oci_client_provider provider;
        std::shared_ptr<oci_client> client;
        image_repository &repository;
        directory_resolver &resolver;
        fs::path image_archive;
        progress_frame frame;
        std::map<std::string, uint16_t> layer_progress;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DOWNLOAD_INSTRUCTION__