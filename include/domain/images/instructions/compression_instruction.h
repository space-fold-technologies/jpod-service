#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COMPRESSION_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COMPRESSION_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <domain/images/payload.h>
#include <memory>
#include <map>
#include <filesystem>
#include <system_error>

namespace spdlog
{
    class logger;
};

struct zip;
typedef zip zip_t;

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
        std::error_code fetch_error_code();
        static void on_progress_update(zip_t *zip_ctx, double progress, void *user_data);

    private:
        const std::string &identifier;
        directory_resolver &resolver;
        fs::path image_filesystem_directory;
        fs::path image_folder;
        zip_t *archive_ptr;
        progress_frame frame;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COMPRESSION_INSTRUCTION__