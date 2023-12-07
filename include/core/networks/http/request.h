#ifndef __JPOD_SERVICE_CORE_NETWORKS_HTTP_REQUEST__
#define __JPOD_SERVICE_CORE_NETWORKS_HTTP_REQUEST__

#include <string>
#include <vector>
#include <map>
#include <asio/streambuf.hpp>
#include <ostream>
#include <core/networks/http/uri.h>

namespace core::networks::http
{
    class RequestBuilder;
    class Request
    {
    public:
        Request(
            std::string method,
            URI uri,
            std::vector<uint8_t> data,
            std::string content_type,
            std::map<std::string, std::string> headers);
        ~Request() = default;
        auto host() const -> std::string
        {
            return uri.host;
        };
        auto port() const -> uint16_t
        {
            return uri.port;
        };
        void write(asio::streambuf &buffer);

        static RequestBuilder builder();

    private:
        std::string header(const std::string &method, const URI &uri, const std::string &content_type);

    private:
        std::string method;
        URI uri;
        std::vector<uint8_t> data;
        std::string content_type;
        std::map<std::string, std::string> headers;
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_HTTP_REQUEST__