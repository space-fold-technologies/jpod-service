#ifndef __JPOD_SERVICE_CORE_CONNECTIONS_CONNECTION__
#define __JPOD_SERVICE_CORE_CONNECTIONS_CONNECTION__

#include <vector>
#include <string>
#include <memory>
namespace core::connections
{
    class ConnectionListener;
    class Connection
    {
    public:
        virtual void connect() = 0;
        virtual void write(const std::vector<uint8_t> &payload) = 0;
        virtual void disconnect() = 0;
        virtual void register_callback(std::shared_ptr<ConnectionListener> listener) = 0;
    };
}

#endif // __JPOD_SERVICE_CORE_CONNECTIONS_CONNECTION__