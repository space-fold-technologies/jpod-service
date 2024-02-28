#include <core/connections/connection_acceptor.h>
#include <core/connections/connection.h>
#include <spdlog/spdlog.h>
#include <sole.hpp>
#include <filesystem>

namespace fs = std::filesystem;

namespace core::connections
{
    connection_acceptor::connection_acceptor(
        asio::io_context &context,
        std::string socket_path,
        std::shared_ptr<core::commands::command_handler_registry> command_handler_registry) : context(context), socket_path(std::move(socket_path)),
                                                                                              command_handler_registry(command_handler_registry),
                                                                                              acceptor(context),
                                                                                              logger(spdlog::get("jpod"))
    {
    }

    void connection_acceptor::start()
    {
        if (!acceptor.is_open())
        {
            if (fs::exists(socket_path))
            {
                fs::remove(socket_path);
            }
            asio::local::stream_protocol::endpoint endpoint(socket_path);
            acceptor.open(endpoint.protocol());
            acceptor.set_option(asio::local::stream_protocol::acceptor::reuse_address(true));
            acceptor.bind(endpoint);
            acceptor.listen();
            std::thread t(&connection_acceptor::accept_connection, this);
            t.join();
        }
    }
    void connection_acceptor::stop()
    {
        if (acceptor.is_open())
        {
            acceptor.close();
            // you can go ahead with cleaning out connections that are present
        }
    }
    void connection_acceptor::accept_connection()
    {
        acceptor.async_accept([this](const std::error_code &err, asio::local::stream_protocol::socket socket)
                              {
            if(!err)
            {
                auto identifier = sole::uuid4().str();
                //TODO:: here is where you will create the connection and add it to a map to keep its life time.
                // The connection will also be a std::shared_ptr<connection> to keep things clean
                auto connection = std::make_shared<core::connections::connection>(
                    identifier, 
                    std::move(socket), 
                    command_handler_registry, 
                    [this](const std::string& identifier) {
                        this->connections.erase(identifier);
                    });
                connections.emplace(identifier, connection);
                connection->start();
                this->accept_connection();
            } });
    }
    connection_acceptor::~connection_acceptor()
    {
    }
}