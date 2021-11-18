#ifndef __JPOD__NETWORK_REPOSITORY__
#define __JPOD__NETWORK_REPOSITORY__
#include <optional>

namespace network {
  class Entry;
  class Repository {
  public:
    virtual void add(const Entry &entry) = 0;
    virtual std::optional<Entry> fetch(const std::string &container_identifier) = 0;
    virtual std::vector<Entry> fetch(std::vector<std::string> container_identifiers) = 0;
    virtual void remove(const std::string &container_identifier) = 0;
  };
} // namespace network
#endif // __JPOD__NETWORK_REPOSITORY__