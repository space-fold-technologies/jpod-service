#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_REGISTRATION_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_REGISTRATION_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <memory>
#include <map>
#include <optional>


namespace spdlog
{
    class logger;
};
namespace domain::images
{
    class image_repository;
}
namespace domain::images::instructions
{
    class instruction_listener;
    struct image_properties;
    struct image_registry_query;
    struct image_properties
    {
        std::string name;
        std::string tag;
        std::string entry_point;
        std::map<std::string, std::string> labels;
        std::map<std::string, std::string> env_vars;
        std::string parent_image_order;
    };
    class registration_instruction : public instruction
    {
    public:
        explicit registration_instruction(
            const std::string &identifier,
            image_properties properties,
            image_repository &repository,
            instruction_listener &listener);
        virtual ~registration_instruction();
        void execute() override;

    private:
        std::optional<image_registry_query> resolve_tagged_image_details();

    private:
        const std::string &identifier;
        image_properties properties;
        image_repository &repository;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_REGISTRATION_INSTRUCTION__