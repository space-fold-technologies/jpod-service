#ifndef __JPOD_NETWORK_HANDLER__
#define __JPOD_NETWORK_HANDLER__

#include <core/networking/contracts.h>
#include <optional>
#include <vector>

namespace network {
  class Repository;

  class InstanceHandler {

  public:
    InstanceHandler();
    ~InstanceHandler();
    void add(const Entry &entry);
    void open(const std::string &container_identifier);
    std::optional<Entry> fetch(const std::string &container_identifier);
    std::vector<Entry> fetch(std::vector<std::string> &container_identifiers);
    void remove(const std::string &container_identifier);

  private:
    Repository &repository;
  };

} // namespace network
#endif // __JPOD_NETWORK_HANDLER__