#ifndef __DAEMON_CORE_CONNECTIONS_CONNECTION_ACCEPTOR__
#define __DAEMON_CORE_CONNECTIONS_CONNECTION_ACCEPTOR__

#include <asio/local/stream_protocol.hpp>
#include <asio/io_context.hpp>
#include <map>

namespace spdlog
{
    class logger;
};

namespace core::commands
{
    class command_handler_registry;
}

namespace core::connections
{
    class connection;
    class connection_acceptor
    {
    public:
        connection_acceptor(asio::io_context &context, std::shared_ptr<core::commands::command_handler_registry> command_handler_registry);
        virtual ~connection_acceptor();
        void start();
        void stop();

    private:
        void accept_connection();

    private:
        asio::io_context &context;
        std::shared_ptr<core::commands::command_handler_registry> command_handler_registry;
        std::map<std::string, std::shared_ptr<connection>> connections;
        asio::local::stream_protocol::acceptor acceptor;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_CORE_CONNECTIONS_CONNECTION_ACCEPTOR__