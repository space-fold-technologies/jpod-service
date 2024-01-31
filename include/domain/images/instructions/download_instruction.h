#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DOWNLOAD_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DOWNLOAD_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <domain/images/http/download_destination.h>
#include <domain/images/payload.h>
#include <memory>
#include <string>
#include <optional>
#include <filesystem>

namespace spdlog
{
    class logger;
};

namespace domain::images::http
{
    class client;
}

namespace domain::images
{
    class image_repository;
    class registry;
    class image_meta;
}

namespace fs = std::filesystem;
namespace domain::images::instructions
{
    constexpr std::size_t DOWNLOAD_BUFFER_SIZE = 1024 * 1000;
    class directory_resolver;
    class instruction_listener;

    class download_instruction : public instruction, public http::download_destination, public std::enable_shared_from_this<download_instruction>
    {
    public:
        explicit download_instruction(
            const std::string &identifier,
            const std::string &order,
            http::client &client,
            image_repository &repository,
            directory_resolver &resolver,
            instruction_listener &listener);
        virtual ~download_instruction();
        void execute() override;
        bool is_valid() override;
        std::size_t chunk_size() const override;
        std::size_t write(const std::vector<uint8_t> &data) override;

    private:
        void fetch_image_details(const registry &reg, const std::string &name, const std::string &tag);
        void download_image_filesystem(const registry &reg, const image_meta &details);
        void extract_image_filesystem(const registry &reg, const image_meta &details);
        void save_image_details(const std::string registry_uri, const image_meta &meta);

    private:
        std::string identifier;
        std::string order;
        http::client &client;
        image_repository &repository;
        directory_resolver &resolver;
        fs::path image_archive;
        progress_frame frame;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DOWNLOAD_INSTRUCTION__