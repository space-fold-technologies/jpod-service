#ifndef __DAEMON_DOMAIN_IMAGES_HTTP_REQUEST__
#define __DAEMON_DOMAIN_IMAGES_HTTP_REQUEST__

#include <string>
#include <vector>
#include <map>
#include <asio/streambuf.hpp>
#include <ostream>
#include <domain/images/http/uri.h>



namespace domain::images::http
{
    class request_builder;
    class request
    {
    public:
        explicit request(
            std::string method,
            internal::uri path,
            std::vector<uint8_t> data,
            std::string content_type,
            std::map<std::string, std::string> headers);
        virtual ~request() = default;
        std::string host() const;
        uint16_t port() const;
        void write(asio::streambuf &buffer);
        static request_builder builder();

    private:
        std::string header(const std::string &method, const internal::uri &path, const std::string &content_type);

    private:
        std::string method;
        internal::uri path;
        std::vector<uint8_t> data;
        std::string content_type;
        std::map<std::string, std::string> headers;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_HTTP_REQUEST__