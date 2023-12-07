#ifndef __JPOD_SERVICE_IMAGES_WORK_DIR_INSTRUCTION__
#define __JPOD_SERVICE_IMAGES_WORK_DIR_INSTRUCTION__

#include <images/instruction.h>
#include <core/networks/http/download_components.h>

namespace spdlog
{
    class logger;
};
namespace images
{
    class WorkDirInstruction : public Instruction
    {
    public:
        explicit WorkDirInstruction(
            const std::string &id,
            const std::string &source,
            const std::string &base_directory,
            std::string &current_directory,
            InstructionListener &listener);
        virtual ~WorkDirInstruction();
        std::error_code parse() override;
        void execute() override;

    private:
        const std::string &id;
        const std::string &source;
        const std::string &base_directory;
        std::string &current_directory;
        InstructionListener &listener;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __JPOD_SERVICE_IMAGES_WORK_DIR_INSTRUCTION__