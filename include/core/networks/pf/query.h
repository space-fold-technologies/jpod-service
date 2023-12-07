#ifndef __JPOD_SERVICE_CORE_NETWORKS_PF_IOCTL_QUERY__
#define __JPOD_SERVICE_CORE_NETWORKS_PF_IOCTL_QUERY__
#include <memory>
#include <net/if.h>
#include <net/pfvar.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string>

namespace core::networks::pf {

class Query
{
public:
  Query();
  ~Query();
  bool has_anchor(const std::string &anchor);
  bool has_table(const std::string &table);
  bool has_subnet(const std::string &address);
  bool has_interface(const std::string &interface);

private:
  bool open_device();
  bool close_device();
  int device_identifier;
  u_int32_t read_address(const std::string &address);
  void print_address(u_int32_t address);
};

};// namespace core::networks::pf

#endif// __JPOD_SERVICE_CORE_NETWORKS_PF_IOCTL_QUERY__