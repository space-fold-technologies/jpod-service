#include <cstdlib>
#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <chrono>
#include <thread>
#include <asio/io_context.hpp>
#include <asio/signal_set.hpp>
#include <bootstrap.h>
#if defined __has_include
#if __has_include("spdlog/fmt/bundled/core.h")
#error "bundled fmt within spdlog should not be present"
#endif
#endif
#include <cmrc/cmrc.hpp>
CMRC_DECLARE(resources);

auto main(int argc, char *argv[]) -> int
{
  auto console = spdlog::stdout_color_mt("jpod");
  auto fs = cmrc::resources::get_filesystem();
  auto file = fs.open("configurations/settings.yml");
  auto configuration = core::configurations::read_from_array(std::string(file.begin(), file.end()));
  if (configuration.has_value())
  {
    asio::io_context context;
    bool running = true;
    asio::signal_set signal(context, SIGINT, SIGTERM);

    auto app = std::make_unique<bootstrap>(context, *configuration);
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
    console->info("remote shell started");
    app->start();
    while (running)
    {
      context.run();
    }
  }

  return EXIT_SUCCESS;
}