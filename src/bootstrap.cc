#include <bootstrap.h>
#include <core/connections/connection_acceptor.h>
#include <core/connections/frame.h>
#include <core/sql/data_source.h>
#include <core/commands/command_handler_registry.h>
#include <domain/images/list_handler.h>
#include <domain/images/build_handler.h>
#include <domain/images/import_handler.h>
#include <domain/images/sql_repository.h>
#include <domain/images/http/asio_client.h>
// container headers
#include <domain/containers/virtual_terminal.h>
#include <domain/containers/sql_repository.h>
#include <domain/containers/creation_handler.h>
#include <domain/containers/logging_handler.h>
#include <domain/containers/start_handler.h>
#include <domain/containers/shell_handler.h>
#include <domain/containers/runtime.h>
#include <asio/io_context.hpp>

using namespace domain::images;
using namespace domain::containers;

bootstrap::bootstrap(asio::io_context &context) : context(context),
                                                  registry(std::make_shared<command_handler_registry>()),
                                                  acceptor(std::make_unique<connection_acceptor>(context, registry)),
                                                  data_source(nullptr),
                                                  image_repository(nullptr),
                                                  container_repository(nullptr),
                                                  runtime(nullptr),
                                                  client(nullptr)
{
}
void bootstrap::setup()
{
  client = std::make_shared<http::asio_client>(context, 4);
  registry->add_handler(
      operation_target::image,
      request_operation::list,
      [&](connection &conn) -> std::unique_ptr<command_handler>
      {
        return std::make_unique<list_handler>(conn);
      });
  registry->add_handler(
      operation_target::image,
      request_operation::build,
      [this](connection &conn) -> std::unique_ptr<command_handler>
      {
        return std::make_unique<build_handler>(conn, image_repository, client, context);
      });
  registry->add_handler(
      operation_target::image,
      request_operation::import,
      [this](connection &conn) -> std::unique_ptr<command_handler>
      {
        return std::make_unique<import_handler>(conn, image_repository);
      });
  registry->add_handler(
      operation_target::container,
      request_operation::build,
      [this](connection &conn) -> std::unique_ptr<command_handler>
      {
        creation_configuration cfg{containers_folder, images_folder};
        return std::make_unique<creation_handler>(conn, cfg, container_repository);
      });
  registry->add_handler(
      operation_target::container,
      request_operation::start,
      [this](connection &conn) -> std::unique_ptr<command_handler>
      {
        return std::make_unique<start_handler>(conn, container_repository, runtime, containers_folder);
      });
  registry->add_handler(
      operation_target::container,
      request_operation::shell,
      [this](connection &conn) -> std::unique_ptr<command_handler>
      {
        auto provider = [this](
                            const std::string &identifier,
                            terminal_listener &listener) -> std::unique_ptr<virtual_terminal>
        {
          return {};
        };
        return std::make_unique<shell_handler>(conn, container_repository, provider);
      });
  registry->add_handler(
      operation_target::container,
      request_operation::logs,
      [this](connection &conn) -> std::unique_ptr<command_handler>
      {
        return std::make_unique<logging_handler>(conn, container_repository, runtime);
      });
}
void bootstrap::start()
{
  acceptor->start();
}
void bootstrap::stop()
{
  acceptor->stop();
}
bootstrap::~bootstrap()
{
}