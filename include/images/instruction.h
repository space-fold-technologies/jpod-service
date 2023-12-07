#ifndef __JPOD_SERVICE_IMAGES_INSTRUCTION__
#define __JPOD_SERVICE_IMAGES_INSTRUCTION__

#include <system_error>
#include <fmt/format.h>
#include <images/instruction_listener.h>

namespace images
{
    class Instruction
    {
    public:
        virtual ~Instruction() = default;
        virtual std::error_code parse() = 0;
        virtual void execute() = 0;
    };
}
#endif // __JPOD_SERVICE_IMAGES_INSTRUCTION__