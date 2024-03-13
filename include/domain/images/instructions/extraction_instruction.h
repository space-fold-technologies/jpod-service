#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_EXTRACTION_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_EXTRACTION_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <domain/images/payload.h>
#include <memory>
#include <vector>
#include <optional>
#include <functional>
#include <system_error>
#include <filesystem>

namespace spdlog
{
    class logger;
};

struct archive;

namespace fs = std::filesystem;
namespace domain::images::instructions
{
    class import_resolver;
    class instruction_listener;
    class extraction_instruction : public instruction
    {
        const std::string FILE_SYSTEM_ARCHIVE = "fs.tar.gz";
        const std::size_t FS_BUFFER_SIZE = 1024 * 100;

    public:
        explicit extraction_instruction(const std::string &identifier,
                                        import_resolver &resolver,
                                        instruction_listener &listener);
        virtual ~extraction_instruction();
        void execute() override;

    private:
        std::error_code initialize();

    private:
        const std::string &identifier;
        import_resolver &resolver;
        fs::path image_archive;
        std::unique_ptr<archive, std::function<void(archive *)>> archive_ptr;
        progress_frame frame;
        std::vector<uint8_t> buffer;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_EXTRACTION_INSTRUCTION__