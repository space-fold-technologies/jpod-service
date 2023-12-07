#include <core/connections/ipc/domain_connection.h>
#include <core/connections/connection_listener.h>
#include <spdlog/spdlog.h>
namespace core::connections::ipc
{
    DomainConnection::DomainConnection(asio::local::stream_protocol::socket socket) : socket(std::move(socket)),
                                                                                      listener(nullptr),
                                                                                      logger(spdlog::get("jpod"))
    {
    }
    void DomainConnection::connect()
    {
    }
    void DomainConnection::register_callback(std::shared_ptr<ConnectionListener> listener)
    {
        this->listener = listener;
    }
    void DomainConnection::write(const std::vector<uint8_t> &payload)
    {
    }
    void DomainConnection::disconnect()
    {
    }
}