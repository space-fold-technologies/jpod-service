#ifndef __CORE_CONNECTIONS_WEBSOCKET_CONNECTION_ACCEPTOR__
#define __CORE_CONNECTIONS_WEBSOCKET_CONNECTION_ACCEPTOR__

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <core/connections/ws/ws_properties_listener.h>
#include <map>
#include <memory>

namespace spdlog
{
    class logger;
};
namespace core::connections
{
    class ConnectionAcceptorListener;
}

namespace core::connections::ws
{
    class WebsocketConnection;
    class WebsocketConnectionAcceptor : public PropertiesListener
    {
    public:
        WebsocketConnectionAcceptor(asio::io_context &context, ConnectionAcceptorListener &listener);
        ~WebsocketConnectionAcceptor() = default;
        void start(uint16_t port);
        void stop();
        void on_request(std::shared_ptr<WebsocketConnection> connection, const std::string &path) override;

    private:
        void accept_connection();

    private:
        asio::io_context &context;
        asio::ip::tcp::acceptor acceptor;
        ConnectionAcceptorListener &listener;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __CORE_CONNECTIONS_WEBSOCKET_CONNECTION_ACCEPTOR__