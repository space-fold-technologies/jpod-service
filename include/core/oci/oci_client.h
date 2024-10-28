#ifndef __DAEMON_CORE_OCI_CLIENT__
#define __DAEMON_CORE_OCI_CLIENT__

#include <system_error>
#include <functional>
#include <map>
#include <memory>
#include <filesystem>
#include <deque>
#include <vector>
#include <cstdint>
#include <core/oci/download_task_listener.h>
#include <core/oci/upload_task_listener.h>

namespace fs = std::filesystem;
namespace asio
{
    class io_context;
};

namespace spdlog
{
    class logger;
};

namespace core::http
{
    class http_session;
    class async_client;
};

namespace core::oci
{
    struct registry_credentials;
    struct registry_session;
    struct image_fetch_order;
    struct progress_update;
    class layer_download_task;
    class upload_task;

    struct manifest_request
    {
        std::string name;
        std::string repository;
        std::string registry;
        std::string tag;
        std::string readable_tag;
        std::string token;
        std::string operating_system;
        std::string architecture;
        fs::path destination;
        std::map<std::string, std::string> headers;
    };
    struct configuration_request
    {
        std::string name;
        std::string registry;
        std::string repository;
        std::string token;
    };
    struct image_properties
    {
        std::string digest;
        std::string tag;
        std::string tag_reference;
        std::string registry;
        std::string repository;
        std::string os;
        std::size_t size;
        std::string variant;
        std::string version;
    };
    struct layer
    {
        std::size_t size;
        std::string media_type;
        std::string digest;
    };
    struct resolve_target
    {
        std::string image_digest;
        std::string layer_digest;
        std::string token;
        std::string media_type;
        std::string path;
    };
    struct blob_target
    {
        std::string digest;
        std::string registry;
        std::string repository;
        std::string token;
        std::string image_identifier;
    };

    struct progress_details
    {
        std::string hash;
        uint16_t percentage;
        bool complete;
        std::string location;
    };

    // callback functions
    using session_provider = std::function<std::shared_ptr<core::http::http_session>(const std::string &scheme, const std::string &host)>;
    using authorization_callback = std::function<void(const std::error_code &)>;
    using image_progress_callback = std::function<void(const std::error_code &, const progress_update &, const image_properties &)>;
    using upload_callback = std::function<void(const progress_details &details, std::error_code ec)>;
    using registration_callback = std::function<void(const std::error_code &err, const std::string &digest, const std::string &location)>;
    using confirmation_callback = std::function<void(const std::error_code &err, std::string_view digest)>;
    // life time resolve task
    struct image_details
    {
        std::vector<layer> layers;
        image_properties properties;
        image_progress_callback callback;
        std::vector<uint8_t> configuration;
        std::vector<uint8_t> manifest;
        fs::path destination;
    };

    struct image_upload_order
    {
        std::string image_identifier;
        std::string repository;
        std::string registry;
        std::string tag;
        fs::path config_location;
        std::string config_digest;
        fs::path manifest_location;
        std::string manifest_digest;
        std::map<std::string, fs::path> layers;
    };

    struct blob_upload_order
    {
        std::string image_identifier;
        std::string token;
        std::string digest;
        std::string registry;
        std::string repository;
    };

    struct upload_operation
    {
        std::string registry;
        std::string repository;
        std::string tag;
        std::string manifest_digest;
        fs::path manifest_path;
        upload_callback callback;
    };

    class oci_client : public download_task_listener,
                       public upload_task_listener
    {
    public:
        explicit oci_client(asio::io_context &context, session_provider provider);
        virtual ~oci_client();
        void authorize(const registry_credentials &credentials, authorization_callback callback);
        void fetch_image(const image_fetch_order &order, image_progress_callback callback);
        void upload_image(const image_upload_order &order, upload_callback callback);
        void on_download_started(const std::string &image_digest, const std::string &layer_digest) override;
        void on_download_complete(const std::string &image_digest, const std::string &layer_digest) override;
        void on_download_update(const update_details &details) override;
        void on_download_failure(const std::string &image_digest, const std::string &layer_digest, const std::error_code &error) override;
        void on_upload_started(const std::string &image_identifier, const std::string &digest) override;
        void on_upload_complete(const std::string &image_identifier, const std::string &digest, const std::string& location) override;
        void on_upload_update(const std::string &image_identifier, std::string_view digest, uint16_t current, uint16_t total) override;
        void on_upload_failure(const std::string &image_identifier, const std::string &digest, const std::error_code &error) override;

    private:
        std::error_code base64_encode(const std::string &input, std::string &output);
        void fetch_manifest(const manifest_request &request, image_progress_callback callback);
        void fetch_configuration(std::string digest, image_progress_callback callback);
        void add_configuration(const std::string &digest, const std::vector<uint8_t> &data, image_progress_callback callback);
        void fetch_layers(std::string digest);
        void resolve_layer();
        void fetch_upload_location(std::string_view image_identifier);
        void register_blob(const blob_upload_order &order, registration_callback callback);
        void upload_manifest(std::string image_identifier);

    private:
        asio::io_context &context;
        session_provider provider;
        std::unique_ptr<core::http::async_client> client;
        std::map<std::string, std::map<std::string, std::shared_ptr<layer_download_task>>> download_tasks;
        std::map<std::string_view, std::map<std::string, std::shared_ptr<upload_task>>> upload_tasks;
        std::map<std::string, image_details> images;
        std::map<std::string, configuration_request> configuration_requests;
        std::map<std::string, std::shared_ptr<registry_session>> sessions;
        std::deque<resolve_target> resolve_queue;
        std::map<std::string_view, std::deque<blob_target>> blob_targets;
        std::map<std::string_view, upload_operation> upload_operations;
        std::deque<std::pair<std::string, std::string>> start_sequence;
        std::map<std::string, std::deque<std::string>> upload_sequence;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_CORE_OCI_CLIENT__
