#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_WORK_DIR_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_WORK_DIR_INSTRUCTION__
#include <domain/images/instructions/instruction.h>
#include <filesystem>
#include <functional>

namespace spdlog
{
    class logger;
};

namespace fs = std::filesystem;
namespace domain::images::instructions
{
    class work_dir_instruction : public instruction
    {
    public:
        explicit work_dir_instruction(
            const std::string &identifier,
            const std::string &order,
            fs::path &current_directory,
            instruction_listener &listener);
        virtual ~work_dir_instruction();
        void execute() override;

    private:
        const std::string &identifier;
        const std::string &order;
        fs::path &current_directory;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_WORK_DIR_INSTRUCTION__
