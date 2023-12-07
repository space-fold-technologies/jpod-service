#ifndef __JPOD_SERVICE_CONTAINERS_INSTRUCTION_LISTENER__
#define __JPOD_SERVICE_CONTAINERS_INSTRUCTION_LISTENER__

#include <vector>
#include <system_error>

namespace images
{
    class InstructionListener
    {
    public:
        virtual void on_instruction_runner_initialized(std::string id) = 0;
        virtual void on_instruction_runner_data_received(std::string id, const std::vector<uint8_t> &content) = 0;
        virtual void on_instruction_runner_completion(std::string id, const std::error_code &err) = 0;
    };
}

#endif //__JPOD_SERVICE_CONTAINERS_INSTRUCTION_LISTENER__
