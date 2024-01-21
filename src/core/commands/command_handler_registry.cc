
#include <core/commands/command_handler_registry.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace core::commands
{
    command_handler_registry::command_handler_registry() : logger(spdlog::get("jpod"))
    {
    }
    void command_handler_registry::add_handler(operation_target target, request_operation operation, command_handler_provider provider)
    {
        auto key = fmt::format("{}|{}",
                               operation_string_value(target),
                               request_operation_value(operation));
        logger->info("added provider with key: {}", key);
        providers.emplace(key, provider);
    }
    std::optional<command_handler_provider> command_handler_registry::fetch(operation_target target, request_operation operation)
    {
        auto key = fmt::format("{}|{}",
                               operation_string_value(target),
                               request_operation_value(operation));
        logger->info("fetching provider with key: {}", key);
        if (auto position = providers.find(key); position != providers.end())
        {
            return position->second;
        }
        return std::nullopt;
    }
    command_handler_registry::~command_handler_registry()
    {
    }
}