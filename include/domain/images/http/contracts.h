#ifndef __DAEMON_DOMAIN_IMAGES_HTTP_CONTRACTS__
#define __DAEMON_DOMAIN_IMAGES_HTTP_CONTRACTS__

#include <vector>
#include <cstdint>
#include <string>
#include <system_error>
#include <functional>
#include <domain/images/http/uri.h>
#include <domain/images/http/response.h>
#include <filesystem>

namespace fs = std::filesystem;
namespace domain::images::http
{
    class download_destination;
    struct upload_status
    {
        std::size_t current;
        std::size_t start;
        std::size_t end;
        std::size_t total;
        bool complete;
    };

    struct connection_session
    {
        std::function<void(const std::error_code &, const response &)> callback;
        response initial_response;
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
    typedef std::function<void(std::error_code, download_status)> report_callback;
    typedef std::function<void(std::error_code, const response &)> response_callback;
    typedef std::function<void(std::error_code, const upload_status &)> upload_callback;
    struct file_download
    {
        report_callback callback;
        std::shared_ptr<download_destination> destination;
        internal::uri uri;
        std::map<std::string, std::string> headers;
        download_status status;
        std::size_t chunk_size;
    };
    struct file_upload
    {
        upload_callback callback;
        fs::path path;
        std::string file_name;
        internal::uri uri;
        upload_status status;
        std::size_t chunk_size;
        std::map<std::string, std::string> headers;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_HTTP_CONTRACTS__