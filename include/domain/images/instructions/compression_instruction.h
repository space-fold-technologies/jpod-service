#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COMPRESSION_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COMPRESSION_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <domain/images/payload.h>
#include <system_error>
#include <functional>
#include <filesystem>
#include <memory>
#include <map>

namespace spdlog
{
    class logger;
};

struct archive;
struct archive_entry;
namespace fs = std::filesystem;
namespace domain::images::instructions
{
    class directory_resolver;
    class instruction_listener;
    const double PROGRESSION_PRECISION = 0.5;
    const int OPERATION_FAILED = -1;

    class compression_instruction : public instruction
    {
    public:
        explicit compression_instruction(
            const std::string &identifier,
            directory_resolver &resolver,
            instruction_listener &listener);
        virtual ~compression_instruction();
        void execute() override;

    private:
        std::error_code initialize();

    private:
        const std::string &identifier;
        directory_resolver &resolver;
        fs::path image_filesystem_directory;
        fs::path image_folder;
        std::unique_ptr<archive, std::function<void(archive *)>> archive_ptr;
        std::unique_ptr<archive_entry, std::function<void(archive_entry *)>> archive_entry_ptr;
        progress_frame frame;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COMPRESSION_INSTRUCTION__