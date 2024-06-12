#ifndef __DAEMON_CORE_HTTP_REST_CLIENT__
#define __DAEMON_CORE_HTTP_REST_CLIENT__
#include <string>
#include <map>
#include <vector>
#include <llhttp.h>
#include <system_error>
#include <functional>
#include <asio/ssl.hpp>
#include <asio/streambuf.hpp>
#include <core/http/response.h>

namespace asio
{
    class io_context;
};

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
    struct request_details
    {
        std::string path;
        std::map<std::string, std::string> headers;
        std::string content_type;
        std::vector<uint8_t> content;
    };
    struct response;
    struct ssl_configuration;
    struct request;
    struct connection_session;
    struct http_session;
    using response_callback = std::function<void(const std::error_code &, const response &)>;
    using session_provider = std::function<std::shared_ptr<http_session>(const std::string &scheme, const std::string &host)>;
    using http_headers = std::map<std::string, std::string>;
    using llhttp_free = std::function<void(llhttp_t *instance)>;
    struct connection_session
    {
        response_callback callback;
        response initial_response;
    };
    class rest_client
    {
    public:
        explicit rest_client(session_provider provider);
        virtual ~rest_client();
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
        void shutdown();

    private:
        void write_call(const std::vector<uint8_t> &content, const internal::uri &url);
        std::error_code compose_request(std::vector<uint8_t> &content, internal::uri &url, const std::string &path, const http_headers &headers, std::string method);
        std::error_code compose_request(std::vector<uint8_t> &content, internal::uri &url, const request_details &details, std::string method);
        void handle_redirect(const std::string &location);
        void handle_retry();
        void on_request_failure(const std::error_code &error);
        void on_response_failure(const std::error_code &error);
        void read_response_header();
        void read_body(std::size_t bytes_left_over);
        void read_body();
        void read_event_stream(const std::shared_ptr<asio::streambuf> &buffer_ptr);
        void read_next_chunk();
        void parse_chunks();
        static int on_message_begin(llhttp_t *parser);
        static int on_body(llhttp_t *parser, char const *data, size_t length);
        static int on_message_complete(llhttp_t *parser);

    private:
        session_provider provider;
        asio::streambuf buffer;
        std::shared_ptr<http_session> connection;
        llhttp_t http_parser;
        llhttp_settings_t settings;
        std::vector<uint8_t> raw_response;
        std::vector<uint8_t> last_request;
        connection_session session;
        std::string last_host;
        u_int16_t retry_count;
        uint16_t retry_max;
        bool follow;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif //__DAEMON_CORE_HTTP_REST_CLIENT__