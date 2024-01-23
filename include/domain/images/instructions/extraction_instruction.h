#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_EXTRACTION_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_EXTRACTION_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <domain/images/payload.h>
#include <memory>
#include <vector>
#include <system_error>
#include <filesystem>

namespace spdlog
{
    class logger;
};

struct zip;
typedef zip zip_t;

namespace fs = std::filesystem;
namespace domain::images::instructions
{
    class import_resolver;
    class instruction_listener;
    class extraction_instruction : public instruction
    {
        const std::string FILE_SYSTEM_ARCHIVE = "fs.zip";
        const std::size_t FS_BUFFER_SIZE = 4096 * 5;

    public:
        explicit extraction_instruction(const std::string &identifier,
                                        import_resolver &resolver,
                                        instruction_listener &listener);
        virtual ~extraction_instruction();
        void execute() override;

    private:
        std::error_code initialize();
        std::error_code fetch_error_code();

    private:
        const std::string &identifier;
        import_resolver &resolver;
        fs::path image_archive;
        zip_t *archive_ptr;
        progress_frame frame;
        std::vector<uint8_t> buffer;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_EXTRACTION_INSTRUCTION__