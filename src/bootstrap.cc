#include <bootstrap.h>
#include <core/connections/connection_acceptor.h>
#include <core/connections/frame.h>
#include <core/sql/data_source.h>
#include <core/commands/command_handler_registry.h>
#include <domain/images/list_handler.h>
#include <domain/images/build_handler.h>
#include <domain/images/sql_repository.h>
#include <domain/images/http/asio_client.h>
#include <asio/io_context.hpp>

using namespace domain::images;

bootstrap::bootstrap(asio::io_context &context) : context(context),
                                                  registry(std::make_shared<command_handler_registry>()),
                                                  acceptor(std::make_unique<connection_acceptor>(context, registry)),
                                                  data_source(nullptr),
                                                  image_repository(nullptr),
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