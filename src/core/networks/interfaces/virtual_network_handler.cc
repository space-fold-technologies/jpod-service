#include <core/networks/interfaces/virtual_network_handler.h>
#include <core/networks/interfaces/network_errors.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_vlan_var.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace core::networks::interfaces
{
    bool VirtualNetworkHandler::has_interface(const std::string &name)
    {
        auto socket = open_socket(AF_LOCAL);
        ifreq request;
        memset(&request, 0, sizeof(request));

        strlcpy(request.ifr_name, name.c_str(), sizeof(request.ifr_name));
        if (socket < 0)
        {
            return false;
        }
        bool exists = ioctl(socket, SIOCGIFCAP, &request) >= 0;

        close(socket);
        return exists;
    }
    response VirtualNetworkHandler::create_interface(const std::string &name, const std::string &alternative, const CIDR &cidr)
    {
        int socket = open_socket(AF_INET);
        if (socket < 0)
        {
            return false;
        }
        printf("OPENED SOCKET \n");
        ifreq request;
        if (!add_interface(socket, request, command.name, command.alternate_name))
        {
            printf("WELL THAT DID NOT GO WELL ON THE FIRST TRY \n");
            close(socket);
            return false;
        }
        else if (!set_addresses(socket, command.alternate_name, command.cidr))
        {
            printf("WELL THAT DID NOT GO WELL ON THE SECOND TRY \n");
            close(socket);
            show_current_error();
            return false;
        }
        close(socket);
        return true;
    }
    response VirtualNetworkHandler::create_jail_interface(const int jail_id)
    {
        ifreq request;
        memset(&request, 0, sizeof(request));
        request.ifr_ifru.ifru_jid = jail_id;
        auto socket = open_socket(AF_LOCAL);
        if (socket < 0)
        {
            return false;
        }
        bool created = ioctl(socket, SIOCSIFVNET, &request) >= 0;
        close(socket);
        return created;
    }
    response VirtualNetworkHandler::destroy_interface(const std::string &name)
    {
        ifreq request;
        memset(&request, 0, sizeof(request));

        strlcpy(request.ifr_name, name.c_str(), sizeof(request.ifr_name));
        auto socket = open_socket(AF_LOCAL);
        if (socket < 0)
        {
            return false;
        }
        bool destroyed = ioctl(socket, SIOCIFDESTROY, &request) >= 0;

        close(socket);
        return destroyed;
    }

    int VirtualNetworkHandler::open_socket(const int address_family)
    {
        return socket(address_family, SOCK_DGRAM, 0);
    }
    void VirtualNetworkHandler::convert_address(sockaddr_in *in, size_t size, const std::string &address)
    {
        in->sin_len = size;
        in->sin_family = AF_INET;
        inet_pton(AF_INET, address.c_str(), &in->sin_addr);
        char _ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &in->sin_addr, _ip, INET_ADDRSTRLEN);
        printf("CONVERTED : %s \n", _ip);
    }
    bool VirtualNetworkHandler::add_interface(const int socket, ifreq &request, const std::string &type, const std::string &name)
    {
        memset(&request, 0, sizeof(request));
        strlcpy(request.ifr_name, type.c_str(), sizeof(request.ifr_name));

        printf("ADDING ADDRESS \n");
        return ioctl(socket, SIOCIFCREATE2, &request) == 0 && rename_interface(socket, request, name);
    }
    bool VirtualNetworkHandler::rename_interface(const int socket, ifreq &request, const std::string &name)
    {
        std::string tmp(name);
        request.ifr_ifru.ifru_data = tmp.data();
        printf("RENAMING INTERFACE \n");
        return ioctl(socket, SIOCSIFNAME, &request) >= 0;
    }
    bool VirtualNetworkHandler::set_addresses(int socket, const std::string &name, const CIDR &details)
    {
        ifaliasreq alias_request;
        memset(&alias_request, 0, sizeof(ifaliasreq));

        convert_address((struct sockaddr_in *)&alias_request.ifra_addr, sizeof(struct sockaddr_in), details.address);
        convert_address((struct sockaddr_in *)&alias_request.ifra_broadaddr, sizeof(struct sockaddr_in), details.broadcast);
        convert_address((struct sockaddr_in *)&alias_request.ifra_mask, sizeof(struct sockaddr_in), details.mask);
        strncpy(alias_request.ifra_name, name.c_str(), IFNAMSIZ);
        printf("SETTING ADDRESS FOR DEVICE: %s \n", alias_request.ifra_name);
        return ioctl(socket, SIOCAIFADDR, &alias_request) == 0;
    }
}