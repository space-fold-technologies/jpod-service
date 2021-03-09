#ifndef __JPOD__ADDRESS_MANAGER__
#define __JPOD__ADDRESS_MANAGER__
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/address_v4_range.hpp>
#include <core/networking/repository/store.h>
#include <optional>
#include <string>
#include <vector>

using boost::asio::ip::address_v4_range;
using boost::asio::ip::address;
using boost::asio::ip::address_v4;

namespace networking::addresses {

  class AddressManager {
  public:
    AddressManager(networking::repository::NetworkStore &store, const std::string &cidr, const std::vector<std::string>& pre_excluded_ip_address);
    std::optional<address> fetch_next_available();

  private:
    networking::repository::NetworkStore &store;
    const std::string &cidr;
    const std::vector<std::string>& pre_excluded_ip_address;
    address_v4_range from_cidr(const std::string &cidr_address);
  };
} // namespace networking::addresses
#endif // __JPOD__ADDRESS_MANAGER__
