#ifndef __DAEMON_CORE_COMMANDS_INTERNAL_CONTRACTS__
#define __DAEMON_CORE_COMMANDS_INTERNAL_CONTRACTS__

#include <msgpack/msgpack.hpp>

namespace core::commands
{
    struct success_payload
    {
        std::string message;
        template <class T>
        void pack(T &pack)
        {
            pack(message);
        }
    };

    inline std::vector<uint8_t> pack_success_payload(success_payload &order)
    {
        return msgpack::pack(order);
    }

    struct error_payload
    {
        std::string message;
        template <class T>
        void pack(T &pack)
        {
            pack(message);
        }
    };

    inline std::vector<uint8_t> pack_error_payload(error_payload &order)
    {
        return msgpack::pack(order);
    }

    struct pending_payload
    {
        std::string operation;
        template <class T>
        void pack(T &pack)
        {
            pack(operation);
        }
    };

    inline std::vector<uint8_t> pack_pending_payload(pending_payload &order)
    {
        return msgpack::pack(order);
    }

}

#endif // __DAEMON_CORE_COMMANDS_INTERNAL_CONTRACTS__