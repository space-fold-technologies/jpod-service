#ifndef __DAEMON_DOMAIN_IMAGES_HTTP_CONNECTION__BASE_
#define __DAEMON_DOMAIN_IMAGES_HTTP_CONNECTION__BASE_

#include <string>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>
#include <asio/posix/stream_descriptor.hpp>
#include <system_error>
#include <type_traits>

namespace spdlog
{
    class logger;
};

namespace asio
{
    class io_context;
};

namespace domain::images::http
{
    class request;
    class response;
    struct connection_session;
    struct file_download;
    struct file_upload;
    class lifetime_listener;

    class connection_base
    {
        const int FILE_CHUNK_SIZE = 4096;
    protected:
        explicit connection_base(asio::io_context &context, const std::string &identifier, lifetime_listener &listener);
        void write_call(request req);
        void fetch_file_details(request req);
        void register_file_details(request req);
        asio::ip::tcp::resolver::results_type resolve_endpoints(const std::string &host, uint16_t port);
        virtual asio::ip::tcp::socket &socket() = 0;

    private:
        bool is_ip_address(const std::string &ip);
        // file upload specific operations
        void read_file_chunk();
        void send_file_chunk(request& req, std::size_t bytes_to_transfer);

        //end file upload operations
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
        void on_request_failure(const std::error_code &error);
        void on_download_failure(const std::error_code &error);
        void on_upload_failure(const std::error_code &error);
        

    protected:
        std::unique_ptr<connection_session> session;
        std::unique_ptr<file_download> download_ptr;
        std::unique_ptr<file_upload> upload_ptr;
        std::shared_ptr<spdlog::logger> logger;

    private:
        const std::string &identifier;
        lifetime_listener &listener;
        asio::ip::tcp::resolver resolver;
        asio::posix::stream_descriptor file_stream;
        asio::streambuf buffer;
        std::vector<uint8_t> file_chunk_buffer;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_HTTP_CONNECTION__BASE_