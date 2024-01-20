#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_CLEANUP_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_CLEANUP_INSTRUCTION__
#include <domain/images/instructions/instruction.h>
#include <memory>
#include <map>
#include <vector>

namespace spdlog
{
    class logger;
};

namespace domain::images::instructions
{    class directory_resolver;
    class instruction_listener;

    class cleanup_instruction : public instruction
    {
    public:
        explicit cleanup_instruction(
            const std::string &identifier,
            std::vector<std::string> targets,
            directory_resolver &resolver,
            instruction_listener &listener);
        virtual ~cleanup_instruction();
        void execute() override;

    private:
        const std::string &identifier;
        std::vector<std::string> targets;
        directory_resolver &resolver;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif //__DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_CLEANUP_INSTRUCTION__