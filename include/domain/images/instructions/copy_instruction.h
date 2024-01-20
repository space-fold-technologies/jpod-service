#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COPY_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COPY_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
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
    const std::size_t BASE_IMAGE_NAME_POSITION = 7;
    class directory_resolver;
    class instruction_listener;
    class copy_instruction : public instruction
    {
    public:
        copy_instruction(
            const std::string &identifier,
            const std::string &order,
            directory_resolver &resolver,
            instruction_listener &listener);
        virtual ~copy_instruction();
        void execute() override;

    private:
        fs::path sanitize_route(const std::string &path, const std::string &target, std::error_code& err);
        std::error_code setup_local_copy_origin(const std::string &order);
        std::error_code setup_stage_copy_origin(const std::string &base, const std::string &order);
        std::error_code setup_destination(const std::string& order);
    private:
        std::string identifier;
        std::string order;
        directory_resolver &resolver;
        fs::path origin;
        fs::path destination;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_COPY_INSTRUCTION__