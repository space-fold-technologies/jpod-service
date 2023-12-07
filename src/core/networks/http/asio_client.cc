#include <core/networks/http/asio_client.h>
#include <core/networks/http/connection.h>
#include <fmt/format.h>

namespace core::networks::http
{
    AsioClient::AsioClient(asio::io_context &context, uint32_t pool_size) : context(context)
    {
        for (auto i = 0; i < pool_size; ++i)
        {
            connections.try_emplace(i, std::make_shared<Connection>(context));
        }
    }
    void AsioClient::execute(const Request &req, std::function<void(const Response &)> cbr)
    {
        connections[0]->execute(req, cbr);
    }

    void AsioClient::download(const std::string &path, std::shared_ptr<Destination> destination, std::function<void(const Status &)> cbr)
    {
        connections[0]->download(path, destination, cbr);
    }
}