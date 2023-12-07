#ifndef __JPOD_SERVICE_CORE_NETWORKS_VIRTUAL_NETWORK_HANDLER__
#define __JPOD_SERVICE_CORE_NETWORKS_VIRTUAL_NETWORK_HANDLER__

#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <tl/expected.hpp>

struct sockaddr_in;
struct ifreq;
typedef tl::expected<std::string, std::error_code> response;
namespace core::networks::interfaces
{

    struct CIDR
    {
        std::string address;
        std::string broadcast;
        std::string mask;
    };

    struct create_command
    {
        std::string name;
        std::string alternate_name;
        CIDR cidr;
    };

    class VirtualNetworkHandler
    {
    public:
        bool has_interface(const std::string &name);
        response create_interface(const std::string &name, const std::string &alternative, const CIDR &cidr);
        response create_jail_interface(const int jail_id);
        response destroy_interface(const std::string &name);

    private:
        int open_socket(const int address_family);
        void convert_address(sockaddr_in *in, size_t size, const std::string &address);
        bool add_interface(const int socket, ifreq &request, const std::string &type, const std::string &name);
        bool rename_interface(const int socket, ifreq &request, const std::string &name);
        bool set_addresses(int socket, const std::string &name, const CIDR &details);
    };
} // namespace core::networks

#endif // __JPOD_SERVICE_CORE_NETWORKS_VIRTUAL_NETWORK_HANDLER__