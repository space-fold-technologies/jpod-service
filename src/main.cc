#include <boost/uuid/uuid_io.hpp>
#include <iostream>
#include <jails/configurations.h>
#include <jails/jail_handler.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <storage/migrations.h>
#include <storage/sql_jail_repository.h>


auto main() -> int {
  auto console = spdlog::stdout_color_mt("app");
  console->info("JPOD VERSION : {}", 1.0);
  prison::Configuration configuration;
  configuration.host_name = "developer";
  configuration.jail_name = "sample";
  configuration.ips_v4.push_back("127.0.0.1");
  configuration.version = "11.0-p10";
  configuration.path = "/rescue";

  configuration.instructions.push_back({"securelevel", "3"});
  configuration.instructions.push_back({"devfs_ruleset", "4"});
  configuration.instructions.push_back({"persist", ""});
  configuration.instructions.push_back({"allow.raw_sockets", ""});
  prison::Migration migration(std::string(":memory:"), "../../migrations");
  migration.migration();
  prison::SqlPrisonRepository prisonRepository(":memory:");

  prison::PrisonHandler prison_handler(prisonRepository);

  auto result = prison_handler.create(configuration);
  if (result.isOk()) {
     auto summary = result.unwrap();
     console->info("JAIL REFERENCE {}", boost::uuids::to_string(summary.reference));
   } else if (result.isErr()) {
     std::string msg = result.unwrapErr();

     console->error("ERR : {}", msg);
   }
  return EXIT_SUCCESS;
}
