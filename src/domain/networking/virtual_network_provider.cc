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
    bool virtual_network_provider::has_interface(const std::string &name, std::error_code &error)
    {
        if (auto socket = open_socket(AF_LOCAL, error); error)
        {
            error = std::error_code(errno, std::system_category());
            return false;
        }
        else
        {
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
    }
    bool virtual_network_provider::create_interface(const std::string &name, const std::string &alternative, const std::vector<ip_address> &addresses, std::error_code &error)
    {
        if (auto socket = open_socket(AF_INET, error); socket < 0)
        {
            error = std::error_code(errno, std::system_category());
            return false;
        }
        else
        {
            if (!add_interface(socket, name, alternative, error))
            {
                close(socket);
                return false;
            }
            else if (!set_addresses(socket, alternative, addresses, error))
            {
                close(socket);
                return false;
            }
            close(socket);
        }
        return true;
    }
    bool virtual_network_provider::create_jail_interface(const int jail_id, std::error_code &error)
    {
        ifreq request;
        memset(&request, 0, sizeof(request));
        request.ifr_ifru.ifru_jid = jail_id;
        if (auto socket = open_socket(AF_LOCAL, error); error)
        {
            error = std::error_code(errno, std::system_category());
            return false;
        }
        else if (ioctl(socket, SIOCSIFVNET, &request) < 0)
        {
            error = std::error_code(errno, std::system_category());
        }
        else
        {
            close(socket);
        }

        return !error;
    }
    bool virtual_network_provider::destroy_interface(const std::string &name, std::error_code &error)
    {
        ifreq request;
        memset(&request, 0, sizeof(request));

        strlcpy(request.ifr_name, name.c_str(), sizeof(request.ifr_name));
        if (auto socket = open_socket(AF_LOCAL, error); error)
        {
            error = std::error_code(errno, std::system_category());
            return false;
        }
        else if (ioctl(socket, SIOCIFDESTROY, &request) < 0)
        {
            error = std::error_code(errno, std::system_category());
        }
        else
        {
            close(socket);
        }
        return !error;
    }

    //
    int virtual_network_provider::open_socket(const int address_family, std::error_code &error)
    {
        if (auto fd = socket(address_family, SOCK_DGRAM, 0); fd == -1)
        {
            error = std::error_code(errno, std::system_category());
            return -1;
        }
        else
        {
            return fd;
        }
    }
    void virtual_network_provider::convert_address(sockaddr_in *in, size_t size, const std::string &address)
    {
        in->sin_len = size;
        in->sin_family = AF_INET;
        inet_pton(AF_INET, address.c_str(), &in->sin_addr);
        char _ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &in->sin_addr, _ip, INET_ADDRSTRLEN);
        logger->info("CONVERTED : {}", _ip);
    }
    bool virtual_network_provider::add_interface(const int socket, const std::string &type, const std::string &name, std::error_code &error)
    {
        ifreq request;
        memset(&request, 0, sizeof(request));
        strlcpy(request.ifr_name, type.c_str(), sizeof(request.ifr_name));

        logger->trace("ADDING ADDRESS");
        if (ioctl(socket, SIOCIFCREATE2, &request) == -1)
        {
            error = std::error_code(errno, std::system_category());
            return false;
        }
        else if (rename_interface(socket, request, name, error) == -1)
        {
            return false;
        }
        return true;
    }
    bool virtual_network_provider::rename_interface(const int socket, ifreq &request, const std::string &name, std::error_code &error)
    {

        std::string tmp(name);
        request.ifr_ifru.ifru_data = tmp.data();
        logger->trace("RENAMING INTERFACE");
        if (ioctl(socket, SIOCSIFNAME, &request) == -1)
        {
            error = std::error_code(errno, std::system_category());
            return false;
        }
        return true;
    }
    bool virtual_network_provider::set_addresses(int socket, const std::string &name, const std::vector<ip_address> &addresses, std::error_code &error)
    {
        for (const auto &address : addresses)
        {
            ifaliasreq alias_request;
            memset(&alias_request, 0, sizeof(ifaliasreq));

            convert_address((struct sockaddr_in *)&alias_request.ifra_addr, sizeof(struct sockaddr_in), address.value);
            convert_address((struct sockaddr_in *)&alias_request.ifra_broadaddr, sizeof(struct sockaddr_in), address.broadcast);
            convert_address((struct sockaddr_in *)&alias_request.ifra_mask, sizeof(struct sockaddr_in), address.netmask);
            strncpy(alias_request.ifra_name, name.c_str(), IFNAMSIZ);
            logger->info("SETTING ADDRESS FOR DEVICE: {}", alias_request.ifra_name);
            if (ioctl(socket, SIOCAIFADDR, &alias_request) - 1)
            {
                error = std::error_code(errno, std::system_category());
                return false;
            }
        }
        return true;
    }
    virtual_network_provider::~virtual_network_provider()
    {
    }
}