#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_VNET_PROVIDER__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_VNET_PROVIDER__

#include <system_error>
#include <string>
#include <vector>

struct sockaddr_in;
struct ifreq;

namespace domain::containers::freebsd
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

    struct create_command
    {
        std::string name;
        uint32_t jail;
        std::vector<ip_address> addresses;
    };

    bool has_interface(const std::string &name, std::error_code &error);
    std::error_code create_interface(const create_command command);
    int open_socket(const int address_family, std::error_code &error);
    bool rename_interface(const int socket, ifreq &request, const std::string &name, std::error_code &error);
    //internal helpers 
    bool create_bridge(uint32_t fd, const std::string &name, std::error_code &error);
    bool add_to_bridge(uint32_t fd, const std::string &name, const std::string& interface);
    bool remove_from_bridge(uint32_t fd, const std::string &name, const std::string& interface);
    // This will be in the network manager
    bool remove_bridge(uint32_t fd, const std::string &name);

}
#endif // __DAEMON_DOMAIN_CONTAINERS_FREEBSD_VNET_PROVIDER__