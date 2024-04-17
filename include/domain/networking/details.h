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
        std::string value;
        std::string netmask;
        std::string broadcast;
        ip_address_type type;
        std::string cidr;
    };

    struct network_entry
    {
        std::string name;
        std::string code;
        std::string subnet;
        std::string scope;
        std::string driver;
    };

    struct network_order
    {
        std::string container;
        std::string bridge;
        std::string code;
        std::string ip;
        ip_address_type type;
        std::string broadcast;
        std::string netmask;
        std::string gateway;
    };

    struct bridge_order
    {
        std::string name;
        std::string ip;
        std::string broadcast;
        ip_address_type type; 
        std::string netmask;
    };

    struct bridge_result
    {
        std::string code;
        std::string container;
        std::string members;
    };
    struct network_membership
    {
        std::string code;
        std::string driver;
        std::string members;
    };
}
#endif // __DAEMON_DOMAIN_NETWORKING_DETAILS__