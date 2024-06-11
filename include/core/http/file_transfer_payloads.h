#ifndef __DAEMON_CORE_HTTP_FILE_TRANSFER_PAYLOADS__
#define __DAEMON_CORE_HTTP_FILE_TRANSFER_PAYLOADS__

#include <core/http/uri.h>
#include <vector>
#include <cstdint>
#include <string>
#include <system_error>
#include <functional>
#include <filesystem>
#include <optional>
#include <asio/streambuf.hpp>
#include <asio/posix/stream_descriptor.hpp>
#include <memory>
#include <map>

namespace fs = std::filesystem;

namespace core::http
{
    class download_destination;
    struct upload_status
    {
        std::size_t current;
        std::size_t start;
        std::size_t end;
        std::size_t total;
        std::string unit;
        bool complete;
    };

    struct download_status
    {
        std::size_t current;
        std::size_t start;
        std::size_t end;
        std::size_t total;
        std::string unit;
        bool complete;
    };

    struct file_upload
    {
        std::function<void(const std::error_code &, const upload_status &)> callback;
        fs::path path;
        std::string file_name;
        internal::uri uri;
        upload_status status;
        std::size_t chunk_size;
        std::map<std::string, std::string> headers;
        int file_descriptor;
        std::unique_ptr<asio::posix::stream_descriptor> file_stream;
        std::vector<uint8_t> file_chunk_buffer;
    };
    struct chunk_request
    {
        std::size_t start;
        std::size_t end;
        std::size_t size;
    };
    struct file_download
    {
        std::string name;
        report_callback callback;
        std::shared_ptr<download_destination> destination;
        internal::uri uri;
        std::map<std::string, std::string> headers;
        download_status status;
        std::size_t chunk_size;
        asio::streambuf buffer;
        std::string media_type;
        std::size_t size;
        std::optional<chunk_request> current_request;
        std::vector<uint8_t> file_chunk_buffer;
    };

    struct download_request
    {
        std::string name;
        std::string url;
        std::string media_type;
        std::size_t size;
        std::map<std::string, std::string> headers;
        std::shared_ptr<download_destination> sink;
    };

    struct upload_request
    {
        std::string url;
        std::map<std::string, std::string> headers;
        std::string file_name;
        fs::path file_path;
    };
}
#endif // __DAEMON_CORE_HTTP_FILE_TRANSFER_PAYLOADS__