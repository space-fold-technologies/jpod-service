#include <domain/images/instructions/registration_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/details.h>
#include <domain/images/instructions/errors.h>
#include <domain/images/mappings.h>
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
        if (auto result = resolve_tagged_image_details(); !result.has_value())
        {
            listener.on_instruction_complete(identifier, make_error_code(error_code::invalid_order_issued));
        }
        else if (auto details = repository.fetch_image_details(result->registry, result->name, result->tag); !details.has_value())
        {
            listener.on_instruction_complete(identifier, make_error_code(error_code::no_matching_image_found));
        }
        else
        {
            listener.on_instruction_initialized(identifier, name);
            details->identifier = identifier;
            details->name = properties.name;
            details->tag = properties.tag;
            details->entry_point = properties.entry_point;
            details->registry_uri = "localhost";
            details->labels.insert(properties.labels.begin(), properties.labels.end());
            details->env_vars.insert(properties.env_vars.begin(), properties.env_vars.end());
            std::error_code error = repository.save_image_details(*details);
            listener.on_instruction_complete(identifier, error);
        }
    }
    std::optional<image_registry_query> registration_instruction::resolve_tagged_image_details()
    {
        if (properties.parent_image_order.empty())
        {
            return std::nullopt;
        }
        auto order = properties.parent_image_order;
        image_registry_query query{};
        std::string tagged_name = "";
        if (order.find_last_of("/") == std::string::npos)
        {
            query.registry = "default";
        }
        else
        {
            auto position = order.find_last_of("/");
            query.registry = order.substr(0, position);
            tagged_name = order.substr(position + 1);
        }
        if (auto position = tagged_name.find_last_of(":"); position != std::string::npos)
        {
            query.name = tagged_name.substr(0, position);
            query.tag = tagged_name.substr(position + 1);
        }
        else
        {
            query.name = tagged_name;
            query.tag = "latest";
        }
        return std::optional{query};
    }
    registration_instruction::~registration_instruction()
    {
    }
}