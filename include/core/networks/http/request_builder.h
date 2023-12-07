#ifndef __JPOD_SERVICE_CORE_NETWORKS_HTTP_REQUEST_BUILDER__
#define __JPOD_SERVICE_CORE_NETWORKS_HTTP_REQUEST_BUILDER__

#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <tl/expected.hpp>

namespace core::networks::http
{
    class Request;
    struct RequestBuilder
    {

    public:
        RequestBuilder &post();

        RequestBuilder &put();

        RequestBuilder &remove();

        RequestBuilder &get();

        RequestBuilder &address(std::string url);
        RequestBuilder &body(std::vector<uint8_t> content);

        RequestBuilder &add_header(std::string key, std::string value);

        tl::expected<Request, std::string> build();

    private:
        std::string method;
        std::string url;
        std::string content_type;
        std::map<std::string, std::string> headers;
        std::vector<uint8_t> content;
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_HTTP_REQUEST_BUILDER__