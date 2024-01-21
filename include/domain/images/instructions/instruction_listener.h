#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_INSTRUCTION_LISTENER__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_INSTRUCTION_LISTENER__

#include <string>
#include <vector>
#include <system_error>

namespace domain::images::instructions
{
    class instruction_listener
    {
    public:
        virtual void on_instruction_initialized(std::string id, std::string name) = 0;
        virtual void on_instruction_data_received(std::string id, const std::vector<uint8_t> &content) = 0;
        virtual void on_instruction_complete(std::string id, std::error_code err) = 0;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_INSTRUCTION_LISTENER__