#ifndef __JPOD_SERVICE_CORE_NETWORKS_HTTP_CONNECTION__
#define __JPOD_SERVICE_CORE_NETWORKS_HTTP_CONNECTION__

#include <core/networks/http/request.h>
#include <core/networks/http/response.h>
#include <core/networks/http/download_components.h>
#include <asio/ip/tcp.hpp>
#include <asio/io_context.hpp>
#include <asio/streambuf.hpp>
#include <functional>
#include <memory>

namespace spdlog
{
    class logger;
};

namespace core::networks::http
{
    struct Session
    {
        std::function<void(const Response &)> callback;
        Response response;
    };
    struct Download
    {
        std::function<void(const Status &)> callback;
        std::shared_ptr<Destination> destination;
        URI uri;
        Status status;
        std::size_t chunk_size;
    };
    class Connection : public std::enable_shared_from_this<Connection>

    {
    public:
        Connection(asio::io_context &context);
        ~Connection() = default;
        void execute(Request request, std::function<void(const Response &)> callback);
        void download(const std::string &path, std::shared_ptr<Destination> destination, std::function<void(const Status &)> callback);

    private:
        bool is_ip_address(const std::string &ip);
        asio::ip::tcp::resolver::results_type resolve_endpoints(const std::string &host, uint16_t port);
        void write_call(Request request);
        void fetch_file_details(Request request);
        void write_fetch_chunk_request(std::size_t start, std::size_t end, std::size_t chunk_size);
        void fetch_chunk_header(std::size_t start, std::size_t end, std::size_t chunk_size);
        void fetch_chunk_content(std::size_t chunk_size);
        void read_file_details();
        void read_response_header();
        void read_body(std::size_t bytes_left_over);
        void read_body();
        void read_event_stream(const std::shared_ptr<asio::streambuf> &buffer_ptr);
        void read_chunk_size(const std::shared_ptr<asio::streambuf> &chunk_buffer);
        void read_chunk_content(std::size_t bytes_to_transfer, const std::shared_ptr<asio::streambuf> &chunk_buffer);
        void clear_end_of_chunk(std::size_t chunk_size, std::size_t additional_bytes, std::istream &stream, const std::shared_ptr<asio::streambuf> &chunk_buffer);

    private:
        asio::ip::tcp::socket socket;
        asio::ip::tcp::resolver resolver;
        asio::streambuf buffer;
        std::unique_ptr<Session> session;
        std::unique_ptr<Download> download_ptr;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_HTTP_CONNECTION__