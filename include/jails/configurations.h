#ifndef __JPOD_JAIL_CONFIGURATIONS__
#define __JPOD_JAIL_CONFIGURATIONS__

#include <iostream>
#include <json.hpp>
#include <map>
#include <string>
#include <vector>

using nlohmann::json;
namespace prison {

  struct Configuration {
    std::string version;
    std::string path;
    std::string name;
    std::string host_name;
    std::vector<std::string> ips_v4;
    std::vector<std::string> ips_v6;
    std::vector<std::pair<std::string, std::string>> instructions;
  };
  inline void to_json(json &j, const Configuration &configuration) {
    auto instructions = json::array();
    std::for_each(std::begin(configuration.instructions), std::end(configuration.instructions), [&](const auto &instruction) {
      json pair;
      pair[instruction.first] = instruction.second;
      instructions.push_back(pair);
    });
    j = json{
        {"version", configuration.version},
        {"path", configuration.path},
        {"name", configuration.name},
        {"ips_v4", configuration.ips_v4},
        {"ips_v6", configuration.ips_v6},
        {"instructions", instructions}

    };
  }

  inline void from_json(const json &j, Configuration &configuration) {
    j.at("version").get_to(configuration.version);
    j.at("path").get_to(configuration.path);
    j.at("name").get_to(configuration.name);
    j.at("ips_v4").get_to(configuration.ips_v4);
    j.at("ips_v6").get_to(configuration.ips_v6);

    auto instructions = j.at("instructions");

    std::for_each(std::begin(instructions), std::end(instructions), [&configuration](const auto &entry) {
      if (entry.is_object()) {
        std::for_each(entry.items().begin(), entry.items().end(), [&configuration](const auto &item) {
          configuration.instructions.push_back({item.key(), item.value()});
        });
      }
    });
  }
}; // namespace prison

#endif // __JPOD_JAIL_CONFIGURATIONS__
