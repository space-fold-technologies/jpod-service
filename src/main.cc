#include <core/containers/manager.h>
#include <definitions.h>
#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

auto main() -> int {
  auto console = spdlog::stdout_color_mt(LOGGER);
  console->info("JPOD VERSION : {}", 1.0);
  using namespace containers;
  Composition composition;
  composition.hostname = "developer";
  composition.ip_v4_address = "127.0.0.1";
  composition.snapshot_path = "/home/william/jpod/containers/alo";
  ContainerManager manager;
  auto report = manager.create(BaseOS::LINUX, composition);

  console->info("OUT COME {}", report.getLog());
  console->info("JAIL IDENTIFIER {}", report.getIdentifier());
  manager.updateEnvironment(report.getIdentifier(), UpdateType::POST, "FOO=BAR");
  return EXIT_SUCCESS;
}
