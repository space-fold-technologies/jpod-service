#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_IMPORT_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_IMPORT_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <system_error>
#include <functional>
#include <optional>
#include <memory>
#include <vector>

namespace spdlog
{
    class logger;
};

namespace domain::images
{
    class image_repository;
    struct import_details;
}
struct archive;
namespace domain::images::instructions
{
    class import_resolver;
    class instruction_listener;
    class import_instruction : public instruction
    {
        const std::string IMAGE_INFO = "info.yml";
        const std::size_t INFO_BUFFER_SIZE = 4096;

    public:
        explicit import_instruction(const std::string &identifier,
                                    image_repository &repository,
                                    import_resolver &resolver,
                                    instruction_listener &listener);
        virtual ~import_instruction();
        void execute() override;

    private:
        std::error_code initialize();
        std::error_code persist_image_details(const import_details& details, std::size_t file_size);

    private:
        const std::string &identifier;
        image_repository &repository;
        import_resolver &resolver;
        std::unique_ptr<archive, std::function<void(archive *)>> archive_ptr;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_IMPORT_INSTRUCTION__