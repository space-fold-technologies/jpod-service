#ifndef __JPOD_CONTAINER_MANAGER__
#define __JPOD_CONTAINER_MANAGER__

#include <core/containers/data_types.h>
#include <jail.h>
namespace containers {

  class ContainerManager {
  public:
    ContainerReport create(BaseOS baseOS, const Composition &composition);
    ContainerReport updateEnvironment(const std::string &identifier, UpdateType updateType, const std::string &variable);
    ContainerReport startProcess(const std::string &identifier, const std::string &executeCommand);
    ContainerReport stop(const std::string &identifier);
    ContainerReport start(const std::string &identifier);
    ContainerInformation fetchInformationByIdentifier(const std::string &identifier);
    std::vector<ContainerInformation> fetchInformation();

  private:
    void add_parameter(std::vector<jailparam> &parameters, const std::string &key, const std::string &value);
    std::string create_jail(BaseOS baseOS, const Composition &composition);
    std::vector<ContainerInformation> fetch_container_details(std::vector<jailparam> &parameters);
  };
}; // namespace containers

#endif // __JPOD_CONTAINER_MANAGER__