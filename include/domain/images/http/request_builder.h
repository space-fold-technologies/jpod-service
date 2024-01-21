#ifndef __DAEMON_DOMAIN_IMAGES_HTTP_REQUEST_BUILDER__
#define __DAEMON_DOMAIN_IMAGES_HTTP_REQUEST_BUILDER__

#include <map>
#include <vector>
#include <cstdint>
#include <string>
#include <system_error>

namespace domain::images::http
{
    class request;

    struct request_builder
    {
        friend class request;

    public:
        request_builder &post();

        request_builder &put();

        request_builder &remove();

        request_builder &get();

        request_builder &address(std::string url);
        request_builder &body(std::vector<uint8_t> content);

        request_builder &add_header(std::string key, std::string value);

        request build(std::error_code &error);

    private:
        request_builder() = default;

    private:
        std::string method;
        std::string url;
        std::string content_type;
        std::map<std::string, std::string> headers;
        std::vector<uint8_t> content;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_HTTP_REQUEST_BUILDER__