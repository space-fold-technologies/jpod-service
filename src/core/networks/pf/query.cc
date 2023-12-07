#include <core/networks/pf/query.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/pfvar.h>

namespace networking::pf {
Query::Query() {}
Query::~Query() {}
bool Query::has_anchor(const std::string &anchor)
{
}


bool Query::has_table(const std::string &table)
{
}

bool Query::has_subnet(const std::string &address)
{
}

bool Query::has_interface(const std::string &interface)
{
}

bool Query::open_device()
{
  this->device_identifier = open("/dev/pf", O_RDONLY);
  if (device_identifier > 0) {
    return true;
  }
  return false;
}
bool Query::close_device()
{
  if (close(this->device_identifier) == 0) {
    return true;
  }
}

u_int32_t Query::read_address(const std::string &address)
{
  int a, b, c, d;
  sscanf(address.c_str(), "%i.%i.%i.%i", &a, &b, &c, &d);
  return htonl(a << 24 | b << 16 | c << 8 | d);
}
void Query::print_address(u_int32_t address)
{
  address = ntohl(address);
  printf("%d.%d.%d.%d",
    address >> 24 & 255,
    address >> 16 & 255,
    address >> 8 & 255,
    address & 255);
}
}// namespace networking::pf