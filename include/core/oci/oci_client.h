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

namespace fs = std::filesystem;
namespace asio
{
    class io_context;
    namespace ssl
    {
        class context;
    };
};

namespace spdlog
{
    class logger;
};

namespace core::http
{
    class http_session;
    class rest_client;
};

namespace core::oci
{
    struct registry_credentials;
    struct registry_session;
    struct image_fetch_order;
    struct progress_update;
    class layer_download_task;

    struct manifest_request
    {
        std::string name;
        std::string repository;
        std::string registry;
        std::string tag;
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
        std::string registry;
        std::string repository;
        std::string os;
        std::size_t size;
        std::string variant;
        std::string version;
        std::vector<std::string> entry_point;
        std::vector<std::string> command;
        std::map<std::string, std::string> env_vars;
        std::map<std::string, std::string> labels;
        std::map<uint16_t, std::string> exposed_ports;
        std::vector<std::string> volumes;
        std::vector<std::string> layer_diffs;
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

    // callback functions
    using session_provider = std::function<std::shared_ptr<core::http::http_session>(const std::string &scheme, const std::string &host)>;
    using authorization_callback = std::function<void(const std::error_code &)>;
    using image_progress_callback = std::function<void(const std::error_code &, const progress_update &, const image_properties &)>;
    // life time resolve task
    struct image_details
    {
        std::string tag;
        std::string registry;
        std::string repository;
        std::string variant;
        std::string version;
        std::vector<layer> layers;
        image_properties properties;
        image_progress_callback callback;
        std::vector<uint8_t> configuration;
        std::vector<uint8_t> manifest;
        fs::path destination;
    };
    class oci_client : public download_task_listener
    {
    public:
        explicit oci_client(asio::io_context &context, session_provider provider);
        virtual ~oci_client();
        void authorize(const registry_credentials &credentials, authorization_callback callback);
        void fetch_image(const image_fetch_order &order, image_progress_callback callback);
        void on_download_started(const std::string &image_digest, const std::string &layer_digest) override;
        void on_download_complete(const std::string &image_digest, const std::string &layer_digest) override;
        void on_download_update(const update_details &details) override;
        void on_download_failure(const std::string &image_digest, const std::string &layer_digest, const std::error_code &error) override;

    private:
        std::error_code base64_encode(const std::string &input, std::string &output);
        void fetch_manifest(const manifest_request &request, image_progress_callback callback);
        void fetch_configuration(std::string digest, image_progress_callback callback);
        void add_configuration(const std::string &digest, const std::vector<uint8_t> &data, image_progress_callback callback);
        void fetch_layers(std::string digest);
        void resolve_layer();
        
    private:
        asio::io_context &context;
        session_provider provider;
        std::unique_ptr<core::http::rest_client> client;
        std::map<std::string, std::map<std::string, std::shared_ptr<layer_download_task>>> download_tasks;
        std::map<std::string, image_details> images;
        std::map<std::string, configuration_request> configuration_requests;
        std::map<std::string, std::shared_ptr<registry_session>> sessions;
        std::deque<resolve_target> resolve_queue;
        std::deque<std::pair<std::string, std::string>> start_sequence;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_CORE_OCI_CLIENT__