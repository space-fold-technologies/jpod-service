#ifndef __JPOD_NETWORKING_REPOSITORY__
#define __JPOD_NETWORKING_REPOSITORY__

#include <string>
#include <vector>

namespace networking::repository {
  class NetworkStore {
  public:
    virtual std::vector<std::string> fetch_used_addresses() = 0;
  };
} // namespace networking::repository
#endif // __JPOD_NETWORKING_REPOSITORY__