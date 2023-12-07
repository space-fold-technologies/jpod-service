#ifndef __JPOD_SERVICE_CORE_CONNECTIONS_CONNECTION_LISTENER__
#define __JPOD_SERVICE_CORE_CONNECTIONS_CONNECTION_LISTENER__

#include <vector>
#include <system_error>

namespace core::connections
{
    class ConnectionListener
    {
    public:
        virtual void on_open() = 0;
        virtual void on_close() = 0;
        virtual void on_message(const std::vector<uint8_t> &payload) = 0;
        virtual void on_error(const std::error_code &ec) = 0;
    };
}
#endif // __JPOD_SERVICE_CORE_CONNECTIONS_CONNECTION_LISTENER__