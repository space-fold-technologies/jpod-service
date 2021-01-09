#include <algorithm>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <core/containers/manager.h>
#include <definitions.h>
#include <filesystem>
#include <fstream>
#include <spdlog/spdlog.h>
#include <sys/jail.h>
#include <sys/param.h>
#include <unistd.h>
namespace fs = std::filesystem;

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

  ContainerReport ContainerManager::writeToProfile(const std::string &identifier, const std::string &path, const std::string &variable) {
    if (!fs::exists(fs::path(path))) {
      return ContainerReport{ContainerState::STOPPED, identifier, "Environment profile file not found"};
    }
    std::ofstream target_file;
    target_file.open(path, std::ios_base::app);
    target_file << std::endl
                << fmt::format("export {}", variable);
    target_file.close();
    return ContainerReport{ContainerState::STOPPED, identifier, "Environment profile file updated"};
  }

  ContainerReport ContainerManager::makeDirectory(const std::string &identifier, const std::string &path) {
    if (!fs::exists(fs::path(path))) {
      if (fs::create_directories(path)) {
        return ContainerReport{ContainerState::STOPPED, identifier, "Directory created"};
      } else {
        return ContainerReport{ContainerState::STOPPED, identifier, "Directory creation failed"};
      }
    }
    return ContainerReport{ContainerState::STOPPED, identifier, "Directory already exists"};
  }

  ContainerReport ContainerManager::copyIn(const std::string &identifier, const std::string &from, const std::string &to) {
    if (!fs::exists(fs::path(from))) {
      return ContainerReport{ContainerState::STOPPED, identifier, fmt::format("{} does not exist", from)};
    }
    if (!fs::exists(fs::path(to))) {
      return ContainerReport{ContainerState::STOPPED, identifier, fmt::format("{} does not exist", to)};
    }
    std::error_code error_code;
    if (fs::is_directory(fs::path(from))) {
      fs::copy(fs::path(from), fs::path(to), fs::copy_options::recursive, error_code);
      if (error_code) {
        return ContainerReport{ContainerState::STOPPED, identifier, fmt::format("Copy failed {}:{}", error_code.value(), error_code.message())};
      }
    } else {
      fs::copy(fs::path(from), fs::path(to), error_code);
      if (error_code) {
        return ContainerReport{ContainerState::STOPPED, identifier, fmt::format("Copy failed {}:{}", error_code.value(), error_code.message())};
      }
    }
    return ContainerReport{ContainerState::STOPPED, identifier, fmt::format("Copied {} to {}", from, to)};
  }

  ContainerReport ContainerManager::stop(const std::string &identifier) {
    int jail_id = jail_getid(identifier.c_str());
    if (jail_id < 0) {
      return ContainerReport(ContainerState::STOPPED, identifier, fmt::format("Could not find matching jail for {}", identifier));
    }
    if (fork() == 0) {
      spdlog::get(LOGGER)->info("SHUTTING DOWN JAIL ID {} ALIAS {}", jail_id, identifier);
      jail_remove(jail_id);
    }
    return ContainerReport{ContainerState::STOPPED, identifier, fmt::format("container {} has been stopped", identifier)};
  }

  ContainerInformation ContainerManager::fetchInformationByIdentifier(const std::string &identifier) {
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