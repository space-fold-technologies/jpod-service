#ifndef __DAEMON_DOMAIN_NETWORKING_DETAILS__
#define __DAEMON_DOMAIN_NETWORKING_DETAILS__

#include <string>

namespace domain::networking
{
    enum class ip_address_type
    {
        v4,
        v6
    };
    struct ip_address
    {
        std::string identifier;
        std::string value;
        std::string netmask;
        std::string broadcast;
        ip_address_type type;
        std::string cidr;
    };
}
#endif // __DAEMON_DOMAIN_NETWORKING_DETAILS__