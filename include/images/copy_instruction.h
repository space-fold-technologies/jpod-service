#ifndef __JPOD_SERVICE_IMAGES_COPY_INSTRUCTION__
#define __JPOD_SERVICE_IMAGES_COPY_INSTRUCTION__

#include <images/instruction.h>
#include <memory>
namespace spdlog
{
    class logger;
};
namespace images
{
    class CopyInstruction : public Instruction
    {
    public:
        CopyInstruction(const std::string &id,
                        const std::string &order,
                        const std::string &local_folder,
                        const std::string &destination_folder,
                        InstructionListener &listener);
        virtual ~CopyInstruction();
        std::error_code parse() override;
        void execute() override;

        private:
        std::string sanitize_route(const std::string& path, const std::string& target);
        std::error_code setup_local_copy_origin(const std::vector<std::string>& order);
        std::error_code setup_stage_copy_origin(const std::vector<std::string>& order);

    private:
        const std::string &id;
        const std::string &order;
        const std::string &local_folder;
        const std::string &destination_folder;
        std::string from;
        std::string to;
        InstructionListener &listener;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __JPOD_SERVICE_IMAGES_COPY_INSTRUCTION__