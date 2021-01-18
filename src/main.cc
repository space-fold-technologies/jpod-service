//#include <core/containers/manager.h>
#include <core/networking/routing/rules.h>
#include <core/shell/terminal.h>
#include <definitions.h>
#include <iostream>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// #include <thread>
// #include <utilities/subprocess/basic_types.h>
// #include <utilities/subprocess/environ.h>
// #include <utilities/subprocess/pipe.h>
// #include <utilities/subprocess/process_builder.h>
// #include <utilities/subprocess/shell_utils.h>
// using subprocess::CompletedProcess;
// using subprocess::PipeOption;
// using subprocess::Popen;
// using subprocess::RunBuilder;

auto main(int argc, char *argv[]) -> int {
  auto console = spdlog::stdout_color_mt(LOGGER);
  console->info("JPOD VERSION : {}", 1.0);
  shell::Terminal terminal;
  terminal.on_update([](const std::string &output) {
    fmt::print("GOT OUT PUT");
    fmt::print(output);
  });

  if (terminal.initialize("bash")) {
    console->info("GOT ACTIVATED");
    terminal.run();
  }
  using namespace networking::routing;
  //auto rule = RuleBuilder::builder().action(Action::NAT).direction(Direction::IN).build();

  //networking::routing::RuleBuilder::builder().action()
  // char buffer[1024];
  // Popen popen = subprocess::RunBuilder({"bash", "-i"})
  //                   .cout(PipeOption::pipe)
  //                   .popen();
  // while (subprocess::pipe_read(popen.cout, buffer, 1024)) {
  //   console->info("result :{}", buffer);
  //   subprocess::pipe_write(popen.cin, "hello world\n", std::strlen("hello world\n"));
  //   popen.close_cin();
  // }
  // popen.close();

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
