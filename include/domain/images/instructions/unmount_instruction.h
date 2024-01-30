#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_UNMOUNT_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_UNMOUNT_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <domain/images/mappings.h>
#include <memory>
#include <string>
#include <system_error>
#include <filesystem>
#include <optional>
namespace spdlog
{
    class logger;
};

namespace domain::images
{
    class image_repository;
}
namespace fs = std::filesystem;
namespace domain::images::instructions
{
    class directory_resolver;
    class instruction_listener;

    class unmount_instruction : public instruction
    {
    public:
        explicit unmount_instruction(
            const std::string &identifier,
            const std::string &order,
            image_repository &repository,
            directory_resolver &resolver,
            instruction_listener &listener);
        virtual ~unmount_instruction();
        void execute() override;

    private:
        std::error_code unmount_filesystems(const std::vector<mount_point> &mount_points, fs::path &directory);

    private:
        std::string identifier;
        std::string order;
        image_repository &repository;
        directory_resolver &resolver;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_UNMOUNT_INSTRUCTION__