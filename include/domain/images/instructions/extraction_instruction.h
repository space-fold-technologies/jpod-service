#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_EXTRACTION_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_EXTRACTION_INSTRUCTION__
#include <domain/images/instructions/instruction.h>
#include <tl/expected.hpp>
#include <memory>
#include <vector>
#include <system_error>
#include <filesystem>
namespace spdlog
{
    class logger;
};

namespace fs = std::filesystem;
namespace domain::images::instructions
{
    using path_result = tl::expected<fs::path, std::error_code>;
    class directory_resolver;
    class instruction_listener;
    class extraction_instruction : public instruction
    {
    public:
        extraction_instruction(
            const std::string &identifier,
            const std::string &order,
            fs::path local_folder,
            directory_resolver &resolver,
            instruction_listener &listener);
        virtual ~extraction_instruction();
        void execute() override;
    private:
        path_result sanitize_route(const std::string &path, const std::string &target);
        path_result setup_destination(const std::string& order);
    private:
        std::string identifier;
        std::string order;
        fs::path local_folder;
        directory_resolver &resolver;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_EXTRACTION_INSTRUCTION__