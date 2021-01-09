#ifndef __JPOD__NETWORK_CONTRACTS__
#define __JPOD__NETWORK_CONTRACTS__
#include <string>
#include <vector>

namespace network {
  struct Entry {
    const std::string &container_identifier;
    const std::string ip_address;
    const std::vector<int> ports;
  };
} // namespace network
#endif // __JPOD__NETWORK_CONTRACTS__