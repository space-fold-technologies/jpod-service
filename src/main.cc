//#include <core/containers/manager.h>
#include <core/networking/routing/rules.h>
#include <definitions.h>
#include <iostream>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

auto main(int argc, char *argv[]) -> int {
  auto console = spdlog::stdout_color_mt(LOGGER);
  console->info("JPOD VERSION : {}", 1.0);
  using namespace networking::routing;
  auto rule = RuleBuilder::builder()
                  .action(Action::NAT)
                  .address_family(AddressFamily::IP_V4)
                  .direction(Direction::IN_OUT)
                  .source("192.168.40.236", 8500)
                  .destination("172.16.24.23", 8500)
                  .build();

  // using namespace containers;
  // Composition composition;
  // composition.hostname = "developer";
  // composition.ip_v4_address = "127.0.0.1";
  // composition.snapshot_path = "/home/william/jpod/containers/alo";
  // ContainerManager manager;
  // auto report = manager.create(BaseOS::LINUX, composition);

  // console->info("OUT COME {}", report.getLog());
  // console->info("JAIL IDENTIFIER {}", report.getIdentifier());

  return EXIT_SUCCESS;
}
