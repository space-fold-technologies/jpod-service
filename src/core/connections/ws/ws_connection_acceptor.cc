#include <core/connections/ws/ws_connection_acceptor.h>
#include <core/connections/ws/ws_connection.h>
#include <core/connections/connection.h>
#include <core/connections/connection_acceptor_listener.h>
#include <spdlog/spdlog.h>
#include <sole.hpp>
#include <thread>

namespace core::connections::ws
{

    WebsocketConnectionAcceptor::WebsocketConnectionAcceptor(asio::io_context &context, ConnectionAcceptorListener &listener) : context(context),
                                                                                                                                acceptor(context),
                                                                                                                                listener(listener),
                                                                                                                                logger(spdlog::get("jpod"))
    {
    }

    void WebsocketConnectionAcceptor::start(uint16_t port)
    {
        if (!acceptor.is_open())
        {
            asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), static_cast<asio::ip::port_type>(port));
            acceptor.open(endpoint.protocol());
            acceptor.set_option(asio::socket_base::reuse_address(true));

#if defined(__linux__) && defined(TCP_FASTOPEN)
            const int qlen = 5; // This seems to be the value that is used in other examples.
            std::error_code ec;
            acceptor.set_option(asio::detail::socket_option::integer<IPPROTO_TCP, TCP_FASTOPEN>(qlen), ec);
#endif // End Linux
            acceptor.bind(endpoint);
            acceptor.listen();
            std::thread t(&WebsocketConnectionAcceptor::accept_connection, this);
            t.join();
        }
    }
    void WebsocketConnectionAcceptor::stop()
    {
        if (acceptor.is_open())
        {
            acceptor.close();
            // you can go ahead with cleaning out connections that are present
        }
    }

    void WebsocketConnectionAcceptor::on_request(std::shared_ptr<WebsocketConnection> connection, const std::string &path)
    {
        auto id = sole::uuid4().str();
        if (auto pos = path.find_first_of("logs"); pos != std::string::npos)
        {
            listener.connection_accepted(id, Target::LOGS, std::shared_ptr<Connection>(connection));
        }
        else if (auto pos = path.find_first_of("shell"); pos != std::string::npos)
        {
            listener.connection_accepted(id, Target::SHELL, std::shared_ptr<Connection>(connection));
        }
    }
    void WebsocketConnectionAcceptor::accept_connection()
    {
        acceptor.async_accept([this](const std::error_code &err, asio::ip::tcp::socket socket)
                              {
            if(!err) 
            {
                auto connection = std::make_shared<WebsocketConnection>(std::move(socket), *this);
                connection->initiate();
                this->accept_connection();
            } else {
                this->logger->error("acceptor err: {}", err.message());
            } });
    }
}
