#include <domain/images/instructions/registration_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/errors.h>
#include <domain/images/mappings.h>
#include <domain/images/helpers.h>
#include <domain/images/repository.h>
#include <spdlog/spdlog.h>
namespace domain::images::instructions
{
    registration_instruction::registration_instruction(
        const std::string &identifier,
        image_properties properties,
        image_repository &repository,
        instruction_listener &listener) : instruction("REGISTRATION", listener),
                                          identifier(identifier),
                                          properties(properties),
                                          repository(repository),
                                          logger(spdlog::get("jpod"))
    {
    }

    void registration_instruction::execute()
    {
        if (auto result = resolve_tagged_image_details(properties.parent_image_order); !result.has_value())
        {
            listener.on_instruction_complete(identifier, make_error_code(error_code::invalid_order_issued));
        }
        else if (auto details = repository.fetch_image_details(result->registry, result->repository, result->tag); !details.has_value())
        {
            listener.on_instruction_complete(identifier, make_error_code(error_code::no_matching_image_found));
        }
        else
        {
            // listener.on_instruction_initialized(identifier, name);
            // details->identifier = identifier;
            // details->name = properties.name;
            // details->tag = properties.tag;
            // details->entry_point = properties.entry_point;
            // details->registry_path = "localhost";
            // details->labels.insert(properties.labels.begin(), properties.labels.end());
            // details->env_vars.insert(properties.env_vars.begin(), properties.env_vars.end());
            // std::error_code error = repository.save_image_details(*details);
            // listener.on_instruction_complete(identifier, error);
        }
    }

    registration_instruction::~registration_instruction()
    {
    }
}