#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_MOUNT_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_MOUNT_INSTRUCTION__

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
#if defined(__FreeBSD__) || defined(BSD) && !defined(__APPLE__)
struct iovec;
#endif
namespace fs = std::filesystem;
namespace domain::images::instructions
{
    class directory_resolver;
    class instruction_listener;
    struct mount_point_entry
    {
        std::string filesystem;
        std::filesystem::path folder;
        std::string options;
        uint64_t flags;
    };
    class mount_instruction : public instruction
    {
    public:
        explicit mount_instruction(
            const std::string &identifier,
            const std::string &order,
            image_repository &repository,
            directory_resolver &resolver,
            instruction_listener &listener);
        virtual ~mount_instruction();
        void execute() override;

    private:
#if defined(__FreeBSD__) || defined(BSD) && !defined(__APPLE__)
        void add_mount_point_entry(std::vector<iovec> &entries, const std::string &key, const std::string &value);
#endif
        bool mount_filesystems(const std::vector<mount_point_entry> &entries, std::error_code &error);
        std::vector<mount_point_entry> resolve_mountpoint_folders(const std::vector<mount_point> &mount_points, std::error_code &error);

    private:
        std::string identifier;
        std::string order;
        image_repository &repository;
        directory_resolver &resolver;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_MOUNT_INSTRUCTION__