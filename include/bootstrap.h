#ifndef __DAEMON_BOOTSTRAP__
#define __DAEMON_BOOTSTRAP__
#include <core/configuration/configuration.h>
#include <domain/containers/terminal_details.h>
#include <domain/networking/details.h>
#include <asio/ssl/context.hpp>
#include <memory>

namespace asio
{
    class io_context;
};

namespace core::commands
{
    class command_handler_registry;
};
namespace core::configurations
{
    struct setting_properties;
};
namespace core::connections
{
    class connection_acceptor;
};

namespace core::sql::pool
{
    class data_source;
};
namespace domain::images
{
    class image_repository;
};
namespace core::http
{
    class http_session;
    struct ssl_configuration;
};

namespace core::oci
{
    class oci_client;
};

namespace domain::containers
{
    class container_repository;
    class terminal_listener;
    class virtual_terminal;
    class container_monitor;
    class runtime;
};

namespace domain::networking
{
    class network_handler;
    class network_service;
    class network_repository;
};

namespace spdlog
{
    class logger;
};

using namespace core::commands;
using namespace core::connections;
using namespace core::configurations;
namespace fs = std::filesystem;
class bootstrap
{
public:
    explicit bootstrap(asio::io_context &context, setting_properties settings);
    virtual ~bootstrap();
    void setup();
    void start();
    void stop();

private:
    

private:
    static asio::ssl::context context_provider();
    void setup_handlers();
    std::unique_ptr<domain::containers::virtual_terminal> create_virtual_terminal(
        domain::containers::terminal_properties properties,
        domain::containers::terminal_listener &listener);
    std::shared_ptr<domain::containers::container_monitor> create_container_monitor();
    std::unique_ptr<domain::networking::network_handler> provide_network_handler();
    std::unique_ptr<core::oci::oci_client> oci_client_provider();
    std::shared_ptr<core::http::http_session> provider_http_session(const std::string &scheme, const std::string &host);

private:
    asio::io_context &context;
    std::shared_ptr<command_handler_registry> registry;
    std::unique_ptr<connection_acceptor> acceptor;
    std::unique_ptr<core::sql::pool::data_source> data_source;
    std::shared_ptr<domain::images::image_repository> image_repository;
    std::shared_ptr<domain::containers::container_repository> container_repository;
    std::shared_ptr<domain::networking::network_repository> network_repository;
    std::shared_ptr<domain::containers::runtime> runtime;
    std::shared_ptr<domain::networking::network_service> network_service;
    // perhaps on

    fs::path containers_folder;
    fs::path images_folder;
    fs::path volumes_folder;
    domain::networking::network_entry default_network_entry;
    std::shared_ptr<spdlog::logger> logger;
};

#endif // __DAEMON_BOOTSTRAP__