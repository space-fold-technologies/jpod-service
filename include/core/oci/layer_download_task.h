#ifndef __DAEMON_CORE_OCI_LAYER_DOWNLOAD_TASK__
#define __DAEMON_CORE_OCI_LAYER_DOWNLOAD_TASK__

#include <core/http/download_destination.h>
#include <memory>
#include <functional>
#include <filesystem>
#include <fstream>
#include <atomic>

namespace fs = std::filesystem;

namespace spdlog
{
    class logger;
};
namespace asio
{
    class io_context;
};
namespace core::http
{
    class file_transfer_client;
    class http_session;
    struct download_status;
};

using session_provider = std::function<std::shared_ptr<core::http::http_session>(const std::string &scheme, const std::string &host)>;

namespace core::oci
{
    constexpr std::size_t DEFAULT_BUFFER = 1024000;
    class download_task_listener;
    struct download_details
    {
        std::string image_digest;
        std::string layer_digest;
        std::size_t layer_size;
        std::string token;
        uint16_t index;
        std::string layer_url;
        std::string media_type;
        fs::path folder;
        session_provider provider;
    };
    class layer_download_task : public core::http::download_destination, public std::enable_shared_from_this<layer_download_task>
    {
    public:
        explicit layer_download_task(asio::io_context &context, download_details details, download_task_listener &listener, std::size_t buffer_size = DEFAULT_BUFFER);
        virtual ~layer_download_task();
        bool is_valid() override;
        std::size_t chunk_size() const override;
        std::size_t write(const std::vector<uint8_t> &data) override;
        void update_target(std::string path);
        void start();
        void abort();

    private:
        void on_download_update(const std::error_code &error, const core::http::download_status &status);

    private:
        asio::io_context &context;
        download_details details;
        std::size_t buffer_size;
        download_task_listener &listener;
        std::string file_path;
        std::unique_ptr<core::http::file_transfer_client> client;
        bool download_started;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_CORE_OCI_LAYER_DOWNLOAD_TASK__