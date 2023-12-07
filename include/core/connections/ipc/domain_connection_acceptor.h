#ifndef __CORE_CONNECTIONS_IPC_DOMAIN_CONNECTION_ACCEPTOR__
#define __CORE_CONNECTIONS_IPC_DOMAIN_CONNECTION_ACCEPTOR__

#include <asio/io_context.hpp>
#include <asio/local/stream_protocol.hpp>
namespace spdlog
{
    class logger;
};
namespace core::connections
{
    class ConnectionAcceptorListener;
}
namespace core::connections::ipc
{
    class DomainConnectionAcceptor
    {
    public:
        DomainConnectionAcceptor(asio::io_context &context, ConnectionAcceptorListener &listener);
        virtual ~DomainConnectionAcceptor() = default;
        void start();
        void stop();

    private:
        void accept_connection();

    private:
        asio::io_context &context;
        asio::local::stream_protocol::acceptor acceptor;
        ConnectionAcceptorListener &listener;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __CORE_CONNECTIONS_IPC_DOMAIN_CONNECTION_ACCEPTOR__
