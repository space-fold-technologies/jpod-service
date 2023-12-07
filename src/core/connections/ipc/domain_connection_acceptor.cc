#include <core/connections/ipc/domain_connection_acceptor.h>
#include <core/connections/ipc/domain_connection.h>
#include <core/connections/connection.h>
#include <core/connections/connection_acceptor_listener.h>
#include <spdlog/spdlog.h>
#include <sole.hpp>

namespace core::connections::ipc
{

    DomainConnectionAcceptor::DomainConnectionAcceptor(asio::io_context &context, ConnectionAcceptorListener &listener) : context(context),
                                                                                                                          acceptor(context),
                                                                                                                          listener(listener),
                                                                                                                          logger(spdlog::get("jpod"))
    {
    }
    void DomainConnectionAcceptor::start()
    {
        if (!acceptor.is_open())
        {
            asio::local::stream_protocol::endpoint endpoint("unix.socket.jpod");
            //             acceptor.open(endpoint.protocol());
            //             acceptor.set_option(asio::socket_base::reuse_address(true));

            // #if defined(__linux__) && defined(TCP_FASTOPEN)
            //             const int qlen = 5; // This seems to be the value that is used in other examples.
            //             std::error_code ec;
            //             acceptor.set_option(asio::detail::socket_option::integer<IPPROTO_TCP, TCP_FASTOPEN>(qlen), ec);
            // #endif // End Linux
            //             acceptor.bind(endpoint);
            //             acceptor.listen();
            std::thread t(&DomainConnectionAcceptor::accept_connection, this);
            t.join();
        }
    }
    void DomainConnectionAcceptor::stop()
    {
        if (acceptor.is_open())
        {
            acceptor.close();
            // you can go ahead with cleaning out connections that are present
        }
    }
    void DomainConnectionAcceptor::accept_connection()
    {
        acceptor.async_accept([this](const std::error_code &err, asio::local::stream_protocol::socket socket)
                              {
            if(!err)
            {
                auto endpoint = socket.local_endpoint();
                auto path = endpoint.path();
                auto identifier = sole::uuid4().str();
                std::shared_ptr<core::connections::Connection> connection = std::make_shared<core::connections::ipc::DomainConnection>(std::move(socket));
                if(auto pos = path.find_first_of("log-"); pos != std::string::npos)
                { 
                    this->listener.connection_accepted(identifier, core::connections::Target::LOGS, std::move(connection));
                } else if (auto pos = path.find_first_of("shell-"); pos != std::string::npos)
                {
                    this->listener.connection_accepted(identifier, core::connections::Target::SHELL, std::move(connection));
                } else if (auto pos = path.find_first_of("rpc-"); pos != std::string::npos)
                {
                    this->listener.connection_accepted(identifier, core::connections::Target::RPC, std::move(connection));
                } else if (auto pos = path.find_first_of("process-"); pos != std::string::npos)
                {
                    this->listener.connection_accepted(identifier, core::connections::Target::PROCESSES, std::move(connection));
                }
                this->accept_connection();
            } });
    }
}