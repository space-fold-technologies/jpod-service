#include <bootstrap.h>
#include <core/connections/connection_acceptor.h>
#include <core/connections/details.h>
#include <core/sql/data_source.h>
#include <core/sql/migrations.h>
#include <core/commands/command_handler_registry.h>
// core::http
#include <core/http/secure_session.h>
#include <core/http/insecure_session.h>
#include <core/http/ssl_configuration.h>

// core::oci
#include <core/oci/oci_client.h>

#include <domain/images/list_handler.h>
#include <domain/images/build_handler.h>
#include <domain/images/import_handler.h>
#include <domain/images/pull_handler.h>
#include <domain/images/sql_repository.h>

// container headers
#include <domain/containers/in_memory_monitor.h>
#include <domain/containers/virtual_terminal.h>
#include <domain/containers/sql_repository.h>
#include <domain/containers/creation_handler.h>
#include <domain/containers/logging_handler.h>
#include <domain/containers/start_handler.h>
#include <domain/containers/shell_handler.h>
#include <domain/containers/list_handler.h>
#include <domain/containers/stop_handler.h>
#include <domain/containers/removal_handler.h>
#include <domain/containers/runtime.h>

// networking
#include <domain/networking/details.h>
#include <domain/networking/network_service.h>
#include <domain/networking/sql_repository.h>

#include <asio/io_context.hpp>
#include <asio/ssl/context.hpp>

#if defined(__FreeBSD__)
#include <domain/containers/freebsd/freebsd_terminal.h>
#include <domain/networking/freebsd/freebsd_network_handler.h>
#endif

#include <spdlog/spdlog.h>

using namespace domain::images;
using namespace domain::containers;
using namespace std::placeholders;

bootstrap::bootstrap(asio::io_context &context, setting_properties settings) : context(context),
                                                                               registry(std::make_shared<command_handler_registry>()),
                                                                               acceptor(std::make_unique<connection_acceptor>(context, settings.domain_socket, registry)),
                                                                               data_source(std::make_unique<core::sql::pool::data_source>(settings.database_path, settings.pool_size)),
                                                                               image_repository(std::make_shared<domain::images::sql_image_repository>(*data_source)),
                                                                               container_repository(std::make_shared<domain::containers::sql_container_repository>(*data_source)),
                                                                               network_repository(std::make_shared<domain::networking::sql_network_repository>(*data_source)),
                                                                               runtime(nullptr),
                                                                               network_service(nullptr),
                                                                               containers_folder(settings.containers_folder),
                                                                               images_folder(settings.images_folder),
                                                                               default_network_entry{"default", settings.bridge, settings.ip_v4_cidr, "local", "bridge"},
                                                                               logger(spdlog::get("jpod"))
{
}
asio::ssl::context bootstrap::context_provider()
{
  core::http::ssl_configuration configuration{};
  configuration.verify = true;
  return core::http::create_context(configuration);
}
void bootstrap::setup()
{
  data_source->initialize();
  network_service = std::make_shared<domain::networking::network_service>(
      network_repository,
      [this]() -> std::unique_ptr<domain::networking::network_handler>
      {
        return this->provide_network_handler();
      });
  runtime = std::make_shared<domain::containers::runtime>(
      context,
      container_repository,
      [this]() -> std::shared_ptr<domain::containers::container_monitor>
      {
        return this->create_container_monitor();
      },
      [this](const std::string &identifier, const std::string &network) -> std::error_code
      {
        return network_service->join(network, identifier);
      },
      [this](const std::string &identifier, const std::string &network) -> std::error_code
      {
        return network_service->leave(network, identifier);
      });
  core::sql::migration_handler handler(*data_source, "migrations");
  handler.migrate();

  // setup the networks
  if (auto error = network_service->add(default_network_entry); !error)
  {
    setup_handlers();
    data_source->initialize();
  }
  else
  {
    logger->error("failed to start up network services: {}", error.message());
  }
}
void bootstrap::start()
{
  acceptor->start();
}
void bootstrap::stop()
{
  acceptor->stop();
}

void bootstrap::setup_handlers()
{
  // image handlers
  registry->add_handler(
      operation_target::image,
      request_operation::list,
      [&](connection &conn) -> std::shared_ptr<command_handler>
      {
        return std::make_shared<image_list_handler>(conn, image_repository);
      });
  registry->add_handler(
      operation_target::image,
      request_operation::build,
      [this](connection &conn) -> std::shared_ptr<command_handler>
      {
        return std::make_shared<build_handler>(conn, image_repository, std::bind(&bootstrap::oci_client_provider, this), context);
      });
  registry->add_handler(
      operation_target::image,
      request_operation::import,
      [this](connection &conn) -> std::shared_ptr<command_handler>
      {
        return std::make_shared<import_handler>(conn, images_folder, image_repository);
      });
  registry->add_handler(
      operation_target::image,
      request_operation::pull,
      [this](connection &conn) -> std::shared_ptr<command_handler>
      {
        return std::make_shared<pull_handler>(conn, image_repository, std::bind(&bootstrap::oci_client_provider, this), images_folder);
      });
  // container handlers
  registry->add_handler(
      operation_target::container,
      request_operation::build,
      [this](connection &conn) -> std::shared_ptr<command_handler>
      {
        return std::make_shared<creation_handler>(conn, this->containers_folder, this->images_folder, container_repository);
      });
  registry->add_handler(
      operation_target::container,
      request_operation::start,
      [this](connection &conn) -> std::shared_ptr<command_handler>
      {
        return std::make_shared<start_handler>(conn, container_repository, runtime, containers_folder);
      });
  registry->add_handler(
      operation_target::container,
      request_operation::stop,
      [this](connection &conn) -> std::shared_ptr<command_handler>
      {
        return std::make_shared<stop_handler>(conn, container_repository, runtime, containers_folder);
      });
  registry->add_handler(
      operation_target::container,
      request_operation::shell,
      [this](connection &conn) -> std::shared_ptr<command_handler>
      {
        auto provider = [this](
                            terminal_properties properties,
                            terminal_listener &listener) -> std::unique_ptr<virtual_terminal>
        {
          return create_virtual_terminal(std::move(properties), listener);
        };
        return std::make_shared<shell_handler>(conn, container_repository, provider);
      });
  registry->add_handler(
      operation_target::container,
      request_operation::logs,
      [this](connection &conn) -> std::shared_ptr<command_handler>
      {
        return std::make_shared<logging_handler>(conn, container_repository, runtime);
      });
  registry->add_handler(
      operation_target::container,
      request_operation::list,
      [this](connection &conn) -> std::shared_ptr<command_handler>
      {
        return std::make_shared<container_list_handler>(conn, container_repository);
      });
  registry->add_handler(
      operation_target::container,
      request_operation::removal,
      [this](connection &conn) -> std::shared_ptr<command_handler>
      {
        return std::make_shared<container_removal_handler>(conn, runtime, container_repository, containers_folder);
      });
  // networking handlers
}

#if defined(__FreeBSD__)
std::unique_ptr<virtual_terminal> bootstrap::create_virtual_terminal(
    terminal_properties properties,
    terminal_listener &listener)
{
  return std::make_unique<freebsd::freebsd_terminal>(context, std::move(properties), listener);
}

std::unique_ptr<domain::networking::network_handler> bootstrap::provide_network_handler()
{
  return std::make_unique<domain::networking::freebsd::freebsd_network_handler>();
}
#endif
std::shared_ptr<domain::containers::container_monitor> bootstrap::create_container_monitor()
{
  // we will need to implement a container monitor of some kind
  return std::make_shared<in_memory_monitor>();
}
std::unique_ptr<core::oci::oci_client> bootstrap::oci_client_provider()
{
  return std::make_unique<core::oci::oci_client>(context, std::bind(&bootstrap::provider_http_session, this, _1, _2));
}
std::shared_ptr<core::http::http_session> bootstrap::provider_http_session(const std::string &scheme, const std::string &host)
{
  if (scheme == "https")
  {
    return std::make_shared<core::http::secure_http_session>(context, context_provider());
  }
  else
  {
    return std::make_shared<core::http::insecure_http_session>(context);
  }
}
bootstrap::~bootstrap()
{
}