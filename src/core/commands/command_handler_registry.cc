
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
        auto key_ = fmt::format("{}|{}",
                                operation_string_value(target),
                                request_operation_value(operation));
        providers.emplace(key_, provider);
    }
    std::optional<command_handler_provider> command_handler_registry::fetch(operation_target target, request_operation operation)
    {
        auto key_ = fmt::format("{}|{}",
                                operation_string_value(target),
                                request_operation_value(operation));
        if (auto position = providers.find(key_); position != providers.end())
        {
            return position->second;
        }
        return std::nullopt;
    }
    std::string command_handler_registry::key(operation_target target, request_operation operation)
    {
        return fmt::format("{}|{}",
                           operation_string_value(target),
                           request_operation_value(operation));
    }
    command_handler_registry::~command_handler_registry()
    {
    }
}