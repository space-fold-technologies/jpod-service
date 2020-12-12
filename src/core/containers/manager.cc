#include <algorithm>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <core/containers/manager.h>
#include <definitions.h>
#include <spdlog/spdlog.h>
#include <sys/jail.h>
#include <sys/param.h>
#include <unistd.h>

namespace containers {
  ContainerReport ContainerManager::create(BaseOS baseOS, const Composition &composition) {
    auto identifier = create_jail(baseOS, composition);
    if (identifier.empty()) {
      return ContainerReport(ContainerState::STOPPED, "", std::string(jail_errmsg));
    }
    return ContainerReport(ContainerState::CREATED, identifier, "");
  }

  std::string ContainerManager::create_jail(BaseOS baseOS, const Composition &composition) {
    std::vector<jailparam> parameters;
    boost::uuids::random_generator gen;
    boost::uuids::uuid reference = gen();
    auto identifier = boost::uuids::to_string(reference);
    identifier.erase(std::remove(identifier.begin(), identifier.end(), '-'), identifier.end());
    spdlog::get(LOGGER)->info("REFERENCE {}", identifier);
    add_parameter(parameters, std::string("name"), identifier);
    add_parameter(parameters, std::string("host.hostname"), composition.hostname);
    add_parameter(parameters, std::string("path"), composition.snapshot_path);
    add_parameter(parameters, std::string("ip4.addr"), composition.ip_v4_address);
    add_parameter(parameters, std::string("ip6.addr"), composition.ip_v6_address);
    add_parameter(parameters, std::string("persist"), "");
    add_parameter(parameters, std::string("allow.raw_sockets"), "");
    if (baseOS == BaseOS::FREE_BSD) {
      add_parameter(parameters, std::string("securelevel"), "3");
      add_parameter(parameters, std::string("devfs_ruleset"), "4");
    }
    if (baseOS == BaseOS::LINUX) {
      add_parameter(parameters, std::string("allow.mount"), "");
      add_parameter(parameters, std::string("allow.mount.devfs"), "");
      add_parameter(parameters, std::string("allow.mount.procfs"), "");
      add_parameter(parameters, std::string("allow.mount.linprocfs"), "");
      add_parameter(parameters, std::string("allow.mount.linsysfs"), "");
      add_parameter(parameters, std::string("allow.mount.tmpfs"), "");
    }

    int jid = jailparam_set(&parameters[0], parameters.size(), JAIL_CREATE);
    jailparam_free(&parameters[0], parameters.size());
    if (jid == -1) {
      return "";
    }
    return identifier;
  }

  ContainerReport ContainerManager::updateEnvironment(const std::string &identifier, UpdateType updateType, const std::string &variable) {
    int jail_id = jail_getid(identifier.c_str());
    spdlog::get(LOGGER)->info("UPDATE TO JID : {}", jail_id);
    return ContainerReport(ContainerState::UPDATED, identifier, "");
  }

  ContainerReport ContainerManager::stop(const std::string &identifier) {
  }

  ContainerReport ContainerManager::start(const std::string &identifier) {
  }

  ContainerInformation ContainerManager::fetchInformationByIdentifier(const std::string &identifier) {
  }

  std::vector<ContainerInformation> ContainerManager::fetchInformation() {
  }

  std::vector<ContainerInformation> ContainerManager::fetch_container_details(std::vector<jailparam> &parameters) {
    jailparam_get(&parameters[0], parameters.size(), 0);
    std::vector<ContainerInformation> results;
    std::transform(parameters.begin(),
                   parameters.end(),
                   results.begin(),
                   [](const jailparam) -> ContainerInformation {
                     ContainerInformation containerInformation;

                     return containerInformation;
                   });
    return results;
  }

  void ContainerManager::add_parameter(std::vector<jailparam> &parameters, const std::string &key, const std::string &value) {
    auto logger = spdlog::get(LOGGER);
    jailparam parameter;
    if (jailparam_init(&parameter, key.c_str()) != 0) {
      logger->error("FAILED TO INIT KEY:{} VALUE:{} ERR:{}", key, value, jail_errmsg);
      return;
    }

    if (value.empty()) {
      parameters.push_back(parameter);
      return;
    }

    if (jailparam_import(&parameter, value.c_str()) != 0) {
      logger->error("FAILED TO ADD KEY:{} VALUE:{} ERR:{}", key, value, jail_errmsg);
      return;
    }

    parameters.push_back(parameter);
    return;
  }
} // namespace containers