#ifndef __DAEMON_CORE_COMMANDS_INTERNAL_CONTRACTS__
#define __DAEMON_CORE_COMMANDS_INTERNAL_CONTRACTS__

#include <msgpack.hpp>

namespace core::commands
{
    struct success_payload
    {
        std::string message;
        MSGPACK_DEFINE(message);
    };

    inline std::vector<uint8_t> pack_success_payload(const success_payload &order)
    {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, order);
        std::vector<uint8_t> output(buffer.size());
        std::memcpy(output.data(), buffer.data(), buffer.size());
        return output;
    }

    struct error_payload
    {
        std::string message;
        MSGPACK_DEFINE(message);
    };

    inline std::vector<uint8_t> pack_error_payload(const error_payload &order)
    {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, order);
        std::vector<uint8_t> output(buffer.size());
        std::memcpy(output.data(), buffer.data(), buffer.size());
        return output;
    }

    struct progress_payload
    {
        std::string operation;
        std::vector<uint8_t> content;
        MSGPACK_DEFINE(operation, content);
    };

    inline std::vector<uint8_t> pack_progress_payload(const progress_payload &order)
    {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, order);
        std::vector<uint8_t> output(buffer.size());
        std::memcpy(output.data(), buffer.data(), buffer.size());
        return output;
    }

}

#endif // __DAEMON_CORE_COMMANDS_INTERNAL_CONTRACTS__