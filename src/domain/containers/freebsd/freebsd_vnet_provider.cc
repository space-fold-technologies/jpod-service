#include <domain/containers/freebsd/freebsd_vnet_provider.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_vlan_var.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace domain::containers::freebsd
{
    bool has_interface(const std::string &name, std::error_code &error)
    {
        return false;
    }
    std::error_code create_interface(const create_command command)
    {
        return {};
    }
    int open_socket(const int address_family, std::error_code &error)
    {
        return 0;
    }
    bool rename_interface(const int socket, ifreq &request, const std::string &name, std::error_code &error);
    // internal helpers
    bool create_bridge(uint32_t fd, const std::string &name, std::error_code &error)
    {
    }
    bool add_to_bridge(uint32_t fd, const std::string &name, const std::string &interface)
    {
    }
    bool remove_bridge(uint32_t fd, const std::string &name)
    {
    }
    
}