#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <asio/io_context.hpp>
#include <asio/signal_set.hpp>
#include <memory>
#include <core/databases/data_source.h>
#include <containers/container_runtime.h>
#include <containers/sql_container_repository.h>
#include <images/sql_image_repository.h>
#include <ops/application.h>
#if defined __has_include
#if __has_include("spdlog/fmt/bundled/core.h")
#error "bundled fmt within spdlog should not be present"
#endif
#endif
using namespace containers;
using namespace images;
auto main(int argc, char *argv[]) -> int
{
  auto console = spdlog::stdout_color_mt("jpod");
  asio::io_context context;
  bool running = true;
  auto runtime = std::make_shared<ContainerRuntime>(context);
  auto data_source = std::make_shared<DataSource>("metadata.db", 5);
  std::shared_ptr<ContainerRepository> container_repository = std::make_shared<SQLContainerRepository>(data_source);
  std::shared_ptr<ImageRepository> image_repository = std::make_shared<SQLImageRepository>(data_source);
  auto app = std::make_unique<operations::Application>(context, runtime, container_repository, image_repository);
  asio::signal_set signal(context, SIGINT, SIGTERM);
  signal.async_wait(
      [&console, &app, &running](const asio::error_code &error, int signal_number)
      {
        if (error || signal_number == SIGTERM || signal_number == SIGKILL)
        {
          console->info("signal to shutdown received");
          app->stop();
          running = false;
        }
      });
  app->start();
  while (running)
  {
    context.run();
  }

  return EXIT_SUCCESS;
}