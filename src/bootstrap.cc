#include <bootstrap.h>
#include <core/connections/connection_acceptor.h>
#include <core/connections/frame.h>
#include <core/sql/data_source.h>
#include <core/sql/migrations.h>
#include <core/commands/command_handler_registry.h>
#include <domain/images/list_handler.h>
#include <domain/images/build_handler.h>
#include <domain/images/import_handler.h>
#include <domain/images/sql_repository.h>
#include <domain/images/http/asio_client.h>
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
#include <domain/containers/runtime.h>
#include <asio/io_context.hpp>

#if defined(__FreeBSD__)
#include <domain/containers/freebsd/freebsd_terminal.h>
#endif

#include <spdlog/spdlog.h>

using namespace domain::images;
using namespace domain::containers;

bootstrap::bootstrap(asio::io_context &context, setting_properties settings) : context(context),
                                                                               registry(std::make_shared<command_handler_registry>()),
                                                                               acceptor(std::make_unique<connection_acceptor>(context, settings.domain_socket, registry)),
                                                                               data_source(std::make_unique<core::sql::pool::data_source>(settings.database_path, settings.pool_size)),
                                                                               image_repository(std::make_shared<domain::images::sql_image_repository>(*data_source)),
                                                                               container_repository(std::make_shared<domain::containers::sql_container_repository>(*data_source)),
                                                                               runtime(nullptr),
                                                                               client(nullptr),
                                                                               containers_folder(settings.containers_folder),
                                                                               images_folder(settings.images_folder),
                                                                               logger(spdlog::get("jpod"))
{
}
void bootstrap::setup()
{
  data_source->initialize();
  client = std::make_shared<http::asio_client>(context, 4);
  runtime = std::make_shared<domain::containers::runtime>(
      context,
      container_repository,
      [this]() -> std::shared_ptr<domain::containers::container_monitor>
      {
        return this->create_container_monitor();
      });

  core::sql::migration_handler handler(*data_source, "migrations");
  handler.migrate();
  setup_handlers();
  data_source->initialize();
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
        return std::make_shared<build_handler>(conn, image_repository, client, context);
      });
  registry->add_handler(
      operation_target::image,
      request_operation::import,
      [this](connection &conn) -> std::shared_ptr<command_handler>
      {
        return std::make_shared<import_handler>(conn, images_folder, image_repository);
      });
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
                            const std::string &identifier,
                            terminal_listener &listener) -> std::unique_ptr<virtual_terminal>
        {
          return create_virtual_terminal(identifier, listener);
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
}

#if defined(__FreeBSD__)
std::unique_ptr<virtual_terminal> bootstrap::create_virtual_terminal(
    const std::string &identifier,
    terminal_listener &listener)
{
  return std::make_unique<freebsd::freebsd_terminal>(context, identifier, listener);
}
#endif

std::shared_ptr<domain::containers::container_monitor> bootstrap::create_container_monitor()
{
  // we will need to implement a container monitor of some kind
  return std::make_shared<in_memory_monitor>();
}

bootstrap::~bootstrap()
{
}