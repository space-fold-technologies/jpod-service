#ifndef __CORE_CONNECTIONS_WEBSOCKET_PROPERTIES_LISTENER__
#define __CORE_CONNECTIONS_WEBSOCKET_PROPERTIES_LISTENER__

#include <string>
#include <memory>

namespace core::connections::ws
{
    class WebsocketConnection;
    class PropertiesListener
    {
    public:
        virtual void on_request(std::shared_ptr<WebsocketConnection> connection, const std::string &path) = 0;
    };
}

#endif // __CORE_CONNECTIONS_WEBSOCKET_PROPERTIES_LISTENER__