#ifndef __CORE_CONNECTIONS_IPC_DOMAIN_CONNECTION__
#define __CORE_CONNECTIONS_IPC_DOMAIN_CONNECTION__

#include <core/connections/connection.h>
#include <asio/local/stream_protocol.hpp>
#include <memory>

namespace spdlog
{
    class logger;
}
namespace core::connections
{
    class ConnectionListener;
}

namespace core::connections::ipc
{
    class DomainConnection : public Connection, public std::enable_shared_from_this<DomainConnection>
    {
    public:
        DomainConnection(asio::local::stream_protocol::socket socket);
        void connect() override;
        void register_callback(std::shared_ptr<ConnectionListener> listener) override;
        void write(const std::vector<uint8_t> &payload) override;
        void disconnect() override;

    private:
        asio::local::stream_protocol::socket socket;
        std::shared_ptr<ConnectionListener> listener;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif //__CORE_CONNECTIONS_IPC_DOMAIN_CONNECTION__