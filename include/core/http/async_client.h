#ifndef __DAEMON_CORE_HTTP_ASYNC_CLIENT__
#define __DAEMON_CORE_HTTP_ASYNC_CLIENT__

#include <core/http/response.h>
#include <asio/streambuf.hpp>
#include <system_error>
#include <functional>
#include <llhttp.h>
#include <cstdint>
#include <memory>
#include <string>
#include <map>

namespace spdlog
{
    class logger;
};

namespace core::http::internal
{
    class uri;
};

namespace core::http
{
    const std::size_t BUFFER_SIZE = 1024 * 6;
    const uint16_t DEFAULT_RETRIES = 10;
    struct request_details
    {
        std::string path;
        std::map<std::string, std::string> headers;
        std::string content_type;
        std::vector<uint8_t> content;
    };

    struct http_session;
    struct request;

    using response_callback = std::function<void(const std::error_code &, const response &)>;
    using session_provider = std::function<std::shared_ptr<http_session>(const std::string &scheme, const std::string &host)>;
    using http_headers = std::map<std::string, std::string>;

    struct connection_session
    {
        std::string id;
        std::string method;
        std::map<std::string, std::string> headers;
        std::vector<uint8_t> request;
        response_callback callback;
        response initial_response;
        uint16_t retries;
        bool follow;
        std::vector<uint8_t> raw;
    };

    class async_client
    {
    public:
        explicit async_client(session_provider provider, uint16_t retries = DEFAULT_RETRIES);
        virtual ~async_client();
        void post(const request_details &details, response_callback callback);
        void put(const request_details &details, response_callback callback);
        void get(const std::string &path,
                 const http_headers &headers,
                 response_callback callback,
                 bool follow = true);
        void remove(
            const std::string &path,
            const http_headers &headers,
            response_callback callback);
        void head(const std::string &path, const http_headers &headers, response_callback callback, bool follow = false);

    private:
        std::shared_ptr<http_session> connection(const internal::uri &uri);
        std::shared_ptr<http_session> connection(const std::string &id);
        void send_request(const request &req);
        void handle_redirect(const std::string &location);
        void retry_last_request();
        void read_response_header();
        void read_body(std::size_t bytes_left_over);
        void read_body();
        void read_next_chunk();
        void parse_chunks();
        void on_request_failure(const std::error_code &error);
        void on_response_failure(const std::error_code &error);
        static int on_message_begin(llhttp_t *parser);
        static int on_body(llhttp_t *parser, char const *data, size_t length);
        static int on_message_complete(llhttp_t *parser);

    private:
        session_provider provider;
        uint16_t retries;
        std::string current_host;
        connection_session session;
        llhttp_t http_parser;
        llhttp_settings_t settings;
        asio::streambuf buffer;
        std::map<std::string, std::shared_ptr<http_session>> connections;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif //__DAEMON_CORE_HTTP_ASYNC_CLIENT__