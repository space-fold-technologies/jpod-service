#ifndef __DAEMON_CORE_CONNECTIONS_DETAILS__
#define __DAEMON_CORE_CONNECTIONS_DETAILS__

#include <cstdint>
#include <vector>
#include <map>
#include <string>

namespace core::connections
{
    enum class operation_target : std::uint8_t
    {
        image = 0b00000000,
        container = 0b00100000,
        registry = 0b01000000,
        plugin = 0b01100000,
        client = 0b10000000
    };

    inline std::map<operation_target, std::string> operation_target_map =
        {
            {operation_target::image, "image-operation"},
            {operation_target::container, "container-operation"},
            {operation_target::registry, "registry-operation"},
            {operation_target::plugin, "plugin-operation"},
            {operation_target::client, "client-operation"}};

    inline std::string &operation_string_value(operation_target target)
    {
        static std::string unknown_key = "no matching operation target for supplied key";
        if (auto position = operation_target_map.find(target); position != operation_target_map.end())
        {
            return position->second;
        }
        return unknown_key;
    };

    enum class request_operation : std::uint8_t
    {
        list = 0x00,
        details = 0x01,
        removal = 0x02,
        push = 0x03,
        pull = 0x04,
        build = 0x05,
        import = 0x06,
        authorize = 0x07,
        start = 0x08,
        stop = 0x09,
        shell = 0x0A,
        logs = 0x0B

    };

    inline std::map<request_operation, std::string> request_operations_map =
        {
            {request_operation::list, "list-sub-operation"},
            {request_operation::details, "details-sub-operation"},
            {request_operation::removal, "removal-sub-operation"},
            {request_operation::push, "push-sub-operation"},
            {request_operation::pull, "pull-sub-operation"},
            {request_operation::build, "build-sub-operation"},
            {request_operation::import, "import-sub-operation"},
            {request_operation::authorize, "authorize-sub-operation"},
            {request_operation::shell, "shell-sub-operation"},
            {request_operation::start, "start-sub-operation"},
            {request_operation::logs, "logs-sub-operation"}};

    inline std::string &request_operation_value(request_operation operation)
    {
        static std::string unknown_key = "no matching request-operation for the supplied key";
        if (auto position = request_operations_map.find(operation); position != request_operations_map.end())
        {
            return position->second;
        }
        return unknown_key;
    }
    enum class response_operation : std::uint8_t
    {
        success = 0x00,
        failure = 0x01,
        progress = 0x02,
        close_order = 0x03,
        frame = 0x04
    };

    inline std::map<response_operation, std::string> response_operation_map =
        {
            {response_operation::success, "success-response-type"},
            {response_operation::failure, "failure-response-type"},
            {response_operation::progress, "progress-response-type"},
            {response_operation::close_order, "close-order-type"},
            {response_operation::frame, "frame-type"}};

    inline std::string &response_operation_value(response_operation operation)
    {
        static std::string unknown_key = "unknown response operation type";
        if (auto position = response_operation_map.find(operation); position != response_operation_map.end())
        {
            return position->second;
        }
        return unknown_key;
    }
}
#endif // __DAEMON_CORE_CONNECTIONS_DETAILS__