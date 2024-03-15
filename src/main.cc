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
#include <CLI/CLI.hpp>

CMRC_DECLARE(resources);

auto main(int argc, char *argv[]) -> int
{
  CLI::App parser{"jpod-daemon"};
  std::optional<std::string> configuration_path;
  std::optional<core::configurations::setting_properties> configuration;
  parser.add_option("-c,--configuration", configuration_path, "yaml configuration for application");
  CLI11_PARSE(parser, argc, argv);
  auto console = spdlog::stdout_color_mt("jpod");
  std::error_code error;
  if (!configuration_path)
  {
    auto fs = cmrc::resources::get_filesystem();
    auto file = fs.open("configurations/settings.yml");
    configuration = core::configurations::read_from_array(std::string(file.begin(), file.end()));
  }
  else if (configuration = core::configurations::read_from_path(*configuration_path, error); error)
  {
    console->error("ERR: {}", error.message());
    return EXIT_FAILURE;
  }

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
    console->info("setting up");
    app->setup();
    app->start();
    console->info("daemon started");
    while (running)
    {
      context.run();
    }
  }

  return EXIT_SUCCESS;
}