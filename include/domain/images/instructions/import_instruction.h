#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_IMPORT_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_IMPORT_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <system_error>
#include <optional>
#include <memory>
#include <vector>

namespace spdlog
{
    class logger;
};

struct zip;
typedef zip zip_t;
struct zip_file;
typedef zip_file zip_file_t;

namespace domain::images
{
    class image_repository;
    struct import_details;
}

namespace domain::images::instructions
{
    struct details_entry
    {
        zip_file_t *file;
        std::size_t size;
        std::string name;
    };
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
        std::error_code fetch_error_code();
        std::optional<details_entry> fetch_details_entry(std::error_code& error);
        std::optional<import_details> extract_image_details(details_entry& entry, std::error_code &error);


    private:
        const std::string &identifier;
        image_repository &repository;
        import_resolver &resolver;
        zip_t *archive_ptr;
        std::vector<uint8_t> buffer;
        std::vector<uint8_t> chunk;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_IMPORT_INSTRUCTION__