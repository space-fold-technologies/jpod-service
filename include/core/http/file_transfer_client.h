#ifndef __DAEMON_CORE_HTTP_FILE_TRANSFER_CLIENT__
#define __DAEMON_CORE_HTTP_FILE_TRANSFER_CLIENT__

#include <functional>
#include <system_error>
#include <memory>
#include <map>

namespace asio
{
    class io_context;
};
namespace core::http::internal
{
    class uri;
};

namespace spdlog
{
    class logger;
};

namespace core::http
{
    struct http_session;
    struct download_request;
    struct upload_request;
    struct upload_status;
    struct download_status;
    struct file_download;
    struct file_upload;

    using session_provider = std::function<std::shared_ptr<http_session>(const std::string &scheme, const std::string &host)>;
    using upload_callback = std::function<void(const std::error_code &, const upload_status &)>;
    using report_callback = std::function<void(const std::error_code &, const download_status &)>;
    using http_headers = std::map<std::string, std::string>;

    enum download_state
    {
        RANGE_QUERY,
        CHUNK_DOWNLOAD,
        CHUNK_FETCH,
        CHUNK_HEADER_PARSE
    };
    class file_transfer_client
    {
    public:
        explicit file_transfer_client(asio::io_context &context, session_provider provider);
        virtual ~file_transfer_client();
        void download(const download_request &request, report_callback callback);
        void upload(const upload_request &request, upload_callback callback);

    private:
        void open_file();
        void register_file_details();
        void read_file_chunk();
        void send_file_chunk(std::size_t bytes_to_transfer);
        void fetch_file_details();
        void write_request(const std::vector<uint8_t> &content, const internal::uri &url);
        std::error_code compose_request(
            std::vector<uint8_t> &content,
            const internal::uri &url,
            const http_headers &headers,
            std::string method);
        std::error_code compose_request(
            std::vector<uint8_t> &content,
            internal::uri &url,
            const std::string &path,
            const http_headers &headers,
            std::string method);
        void on_server_response(const http_headers &headers, download_state state, const std::string &method);
        void request_partial_content();
        void read_partial_content(std::size_t bytes_to_transfer);
        void handle_retry(const http_headers &headers, download_state state, const std::string &method);
        void handle_redirect(const http_headers &headers, download_state state, const std::string &location, const std::string &method);
        void on_download_failure(const std::error_code &error);
        void on_upload_failure(const std::error_code &error);

    private:
        asio::io_context &context;
        session_provider provider;
        std::unique_ptr<file_download> _download;
        std::unique_ptr<file_upload> _upload;
        std::shared_ptr<http_session> connection;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_CORE_HTTP_FILE_TRANSFER_CLIENT__