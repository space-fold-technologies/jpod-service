#ifndef __JPOD_SERVICE_CORE_NETWORKS_HTTP_ASIO_CLIENT__
#define __JPOD_SERVICE_CORE_NETWORKS_HTTP_ASIO_CLIENT__

#include <core/networks/http/client.h>
#include <asio/io_context.hpp>
#include <memory>
#include <map>

namespace core::networks::http
{
    class Connection;
    constexpr uint32_t BUFFER_SIZE = 8192;
    class AsioClient : public Client
    {
    public:
        AsioClient(asio::io_context &context, uint32_t pool_size = 1);
        void execute(const Request &req, std::function<void(const Response &)> cbr) override;
        void download(const std::string &path, std::shared_ptr<Destination> destination, std::function<void(const Status &)> cbr) override;

    private:
        asio::io_context &context;
        std::map<uint32_t, std::shared_ptr<Connection>> connections;
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_HTTP_ASIO_CLIENT__