#ifndef __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_PAYLOAD__
#define __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_PAYLOAD__
#include <vector>
#include <cstdlib>
namespace core::networks::messages
{
    enum class Type
    {
        CONTAINERS,
        IMAGES,
        NETWORKS,
        RUN,
        EXEC,
        SHELL,
        BUILD,
        PUSH,
        PULL,
        VERSION,
        TOP
    };

    enum class Op
    {
        INFO,
        LIST,
        RM,
        NONE
    };

    struct Payload
    {
        Type type;
        std::size_t content_length;
        std::vector<uint8_t> content;
    };

    inline bool write(std::vector<uint8_t> &buffer, Type type, Op operation, const std::vector<uint8_t> &content)
    {
        return false;
    }
    inline bool parse(Payload &payload, const std::vector<uint8_t> &content)
    {
        return false;
    }
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_PAYLOAD__