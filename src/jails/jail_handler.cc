#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <jails/configurations.h>
#include <jails/jail_handler.h>
#include <jails/jail_repository.h>
#include <jails/models.h>
#include <json.hpp>
#include <spdlog/spdlog.h>
#include <sys/jail.h>
#include <sys/param.h>

namespace prison {
  PrisonHandler::PrisonHandler(PrisonRepository &prisonRepository) : prisonRepository(prisonRepository) {
  }

  Result<Summary, std::string> PrisonHandler::create(const Configuration &configuration) {
    std::vector<jailparam> parameters;
    auto logger = spdlog::get("app");
    boost::uuids::random_generator gen;
    boost::uuids::uuid reference = gen();
    add_parameter(parameters, std::string("name"), to_string(reference));
    add_parameter(parameters, std::string("host.hostname"), configuration.host_name);
    add_parameter(parameters, std::string("path"), configuration.path);
    std::for_each(
        std::begin(configuration.ips_v4),
        std::end(configuration.ips_v4),
        [&](const auto &ip_v4) { add_parameter(parameters, std::string("ip4.addr"), ip_v4); });
    std::for_each(
        std::begin(configuration.ips_v6),
        std::end(configuration.ips_v6),
        [&](const auto &ip_v6) { add_parameter(parameters, std::string("ip6.addr"), ip_v6); });
    std::for_each(
        std::begin(configuration.instructions),
        std::end(configuration.instructions),
        [&](const auto &instruction) { add_parameter(parameters, instruction.first, instruction.second); });

    auto jid = jailparam_set(&parameters[0], parameters.size(), JAIL_CREATE | JAIL_ATTACH);
    jailparam_free(&parameters[0], parameters.size());
    if (jid == -1) {
      return Err(std::string(fmt::format("Failed to create jail err: {}", jail_errmsg)));
    } else {
      jail_attach(jid);
      save_details(jid, to_string(reference), configuration);
    }
    Summary summary = {jid, boost::uuids::to_string(reference), configuration.name};
    return Ok(summary);
  }

  bool PrisonHandler::execute(const std::string &script) {
  }

  void PrisonHandler::remove(int id) {
  }

  std::string PrisonHandler::format_addresses(const std::vector<std::string> &addresses) const {
    if (addresses.empty()) {
      return "";
    }
    //TODO : use stl algorithms to create a comma separated list
    return std::string("");
  }
  bool PrisonHandler::add_parameter(std::vector<jailparam> &parameters, const std::string &key, const std::string &value) {
    auto logger = spdlog::get("app");
    jailparam parameter;
    if (jailparam_init(&parameter, key.c_str()) != 0) {
      logger->error("FAILED TO INIT KEY:{} VALUE:{}", key, value);
      return false;
    }
    if (value.empty()) {
      parameters.push_back(parameter);
      return true;
    }
    if (jailparam_import(&parameter, value.c_str()) != 0) {
      logger->error("FAILED TO ADD KEY:{} VALUE:{}", key, value);
      return false;
    }
    parameters.push_back(parameter);
    return true;
  }

  void PrisonHandler::save_details(int jid, const std::string reference, const Configuration &configuration) {
    // SQL to add jail to database
    spdlog::get("app")->info("JID : {} REFERERENCE {}", jid, reference);
    // Convert all configurations into json
    auto instructions = json::array();
    std::for_each(std::begin(configuration.instructions), std::end(configuration.instructions), [&](const auto &instruction) {
      json pair;
      pair[instruction.first] = instruction.second;
      instructions.push_back(pair);
    });
    json j = configuration;
    Details details;
    details.name = configuration.name;
    details.jid = jid;
    details.reference = reference;
    details.status = "RUNNING";
    details.properties = j.dump();
    prisonRepository.save(details);
  }

  PrisonHandler::~PrisonHandler() {
  }

}; // namespace prison
