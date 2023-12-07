#ifndef __JPOD_SERVICE_CORE_CONNECTIONS_CONNECTION_ACCEPTOR_LISTENER__
#define __JPOD_SERVICE_CORE_CONNECTIONS_CONNECTION_ACCEPTOR_LISTENER__

#include <memory>

namespace core::connections
{
    enum class Target
    {
        LOGS = 0,
        SHELL = 1,
        PROCESSES = 2,
        RPC = 3
    };

    class Connection;
    class ConnectionAcceptorListener
    {
    public:
        virtual void connection_accepted(std::string identifier, Target target, std::shared_ptr<Connection> connection) = 0;
    };
}

#endif // __JPOD_SERVICE_CORE_CONNECTIONS_CONNECTION_ACCEPTOR_LISTENER__