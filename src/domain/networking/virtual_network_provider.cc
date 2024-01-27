#include <domain/networking/virtual_network_provider.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>
#include <net/if_vlan_var.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <spdlog/spdlog.h>

namespace domain::networking
{
    virtual_network_provider::virtual_network_provider() : logger(spdlog::get("jpod"))
    {
    }
    bool virtual_network_provider::has_interface(const std::string &name)
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
    bool virtual_network_provider::create_interface(const std::string &name, const std::string &alternative, const std::vector<ip_address> &addresses, std::error_code &error)
    {
        // int socket = open_socket(AF_INET);
        // if (socket < 0)
        // {
        //     return false;
        // }
        // printf("OPENED SOCKET \n");
        // ifreq request;
        // if (!add_interface(socket, request, name, alternative))
        // {
        //     printf("WELL THAT DID NOT GO WELL ON THE FIRST TRY \n");
        //     close(socket);
        //     return false;
        // }
        // else if (!set_addresses(socket, alternative, command.cidr))
        // {
        //     printf("WELL THAT DID NOT GO WELL ON THE SECOND TRY \n");
        //     close(socket);
        //     //show_current_error();
        //     return false;
        // }
        // close(socket);
        return true;
    }
    bool virtual_network_provider::create_jail_interface(const int jail_id, std::error_code &error)
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
    bool virtual_network_provider::destroy_interface(const std::string &name, std::error_code &error)
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

    //
    int virtual_network_provider::open_socket(const int address_family)
    {
        return socket(address_family, SOCK_DGRAM, 0);
    }
    void virtual_network_provider::convert_address(sockaddr_in *in, size_t size, const std::string &address)
    {
        in->sin_len = size;
        in->sin_family = AF_INET;
        inet_pton(AF_INET, address.c_str(), &in->sin_addr);
        char _ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &in->sin_addr, _ip, INET_ADDRSTRLEN);
        printf("CONVERTED : %s \n", _ip);
    }
    bool virtual_network_provider::add_interface(const int socket, ifreq &request, const std::string &type, const std::string &name)
    {
        memset(&request, 0, sizeof(request));
        strlcpy(request.ifr_name, type.c_str(), sizeof(request.ifr_name));

        printf("ADDING ADDRESS \n");
        return ioctl(socket, SIOCIFCREATE2, &request) == 0 && rename_interface(socket, request, name);
    }
    bool virtual_network_provider::rename_interface(const int socket, ifreq &request, const std::string &name)
    {
        std::string tmp(name);
        request.ifr_ifru.ifru_data = tmp.data();
        printf("RENAMING INTERFACE \n");
        return ioctl(socket, SIOCSIFNAME, &request) >= 0;
    }
    bool virtual_network_provider::set_addresses(int socket, const std::string &name, const std::vector<ip_address> &addresses)
    {
        ifaliasreq alias_request;
        memset(&alias_request, 0, sizeof(ifaliasreq));

        // convert_address((struct sockaddr_in *)&alias_request.ifra_addr, sizeof(struct sockaddr_in), details.address);
        // convert_address((struct sockaddr_in *)&alias_request.ifra_broadaddr, sizeof(struct sockaddr_in), details.broadcast);
        // convert_address((struct sockaddr_in *)&alias_request.ifra_mask, sizeof(struct sockaddr_in), details.mask);
        strncpy(alias_request.ifra_name, name.c_str(), IFNAMSIZ);
        printf("SETTING ADDRESS FOR DEVICE: %s \n", alias_request.ifra_name);
        return ioctl(socket, SIOCAIFADDR, &alias_request) == 0;
    }
    virtual_network_provider::~virtual_network_provider()
    {
    }
}