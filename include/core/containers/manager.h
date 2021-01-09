#ifndef __JPOD_CONTAINER_MANAGER__
#define __JPOD_CONTAINER_MANAGER__

#include <core/containers/data_types.h>
#include <jail.h>
namespace containers {

  class ContainerManager {
  public:
    ContainerReport create(BaseOS baseOS, const Composition &composition);
    ContainerReport writeToProfile(const std::string &identifier, const std::string &path, const std::string &variable);
    ContainerReport makeDirectory(const std::string &identifier, const std::string &path);
    ContainerReport copyIn(const std::string &identifier, const std::string &from, const std::string &to);
    ContainerReport startProcess(const std::string &identifier, const std::string &executeCommand);
    ContainerReport stop(const std::string &identifier);
    ContainerInformation fetchInformationByIdentifier(const std::string &identifier);

  private:
    void add_parameter(std::vector<jailparam> &parameters, const std::string &key, const std::string &value);
    std::string create_jail(BaseOS baseOS, const Composition &composition);
    std::vector<ContainerInformation> fetch_container_details(std::vector<jailparam> &parameters);
  };
}; // namespace containers

#endif // __JPOD_CONTAINER_MANAGER__