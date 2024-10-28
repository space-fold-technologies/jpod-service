#ifndef __DAEMON_CORE_OCI_UPLOAD_TASK__
#define __DAEMON_CORE_OCI_UPLOAD_TASK__

#include <functional>
#include <filesystem>
#include <fstream>
#include <memory>
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
    struct upload_status;
};

using session_provider = std::function<std::shared_ptr<core::http::http_session>(const std::string &scheme, const std::string &host)>;

namespace core::oci
{
    class upload_task_listener;
    struct upload_details
    {
        std::string image_identifier;
        std::string digest;
        fs::path file_path;
        std::string registry;
        std::string repository;
        std::string token;
        uint16_t index;
        session_provider provider;
    };

    class upload_task : public std::enable_shared_from_this<upload_task>
    {
    public:
        explicit upload_task(asio::io_context &context, upload_details details, upload_task_listener &listener);
        virtual ~upload_task();
        void set_location(std::string location);
        void start();
        void abort();
        void confirm_upload();
    private:
        void on_status_update(const std::error_code &error, const core::http::upload_status &status);
    private:
        asio::io_context &context;
        upload_details details;
        upload_task_listener &listener;
        std::unique_ptr<core::http::file_transfer_client> client;
        std::string location;
        bool upload_started;
        std::shared_ptr<spdlog::logger> logger;
    };

}
#endif // __DAEMON_CORE_OCI_UPLOAD_TASK__
