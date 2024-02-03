#ifndef __DAEMON_CORE_COMMANDS_COMMAND_HANDLER_REGISTRY__
#define __DAEMON_CORE_COMMANDS_COMMAND_HANDLER_REGISTRY__
#include <memory>
#include <core/commands/command_handler.h>
#include <core/connections/frame.h>
#include <functional>
#include <map>
#include <optional>

namespace spdlog
{
    class logger;
};

namespace core::connections
{
    class connection;
}

using namespace core::connections;
namespace core::commands
{
    typedef std::function<std::shared_ptr<command_handler>(connection &)> command_handler_provider;
    class command_handler_registry
    {
    public:
        command_handler_registry();
        virtual ~command_handler_registry();
        void add_handler(operation_target target, request_operation operation, command_handler_provider provider);
        std::optional<command_handler_provider> fetch(operation_target target, request_operation operation);

    private:
        std::map<std::string, command_handler_provider> providers;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_CORE_COMMANDS_COMMAND_HANDLER_REGISTRY__