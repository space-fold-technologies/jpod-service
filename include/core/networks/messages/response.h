#ifndef __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_RESPONSE__
#define __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_RESPONSE__

#include <vector>

namespace core::networks::messages
{
    class Response
    {
    public:
        void write(const std::vector<uint8_t>& content);
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_MESSAGES_RESPONSE__