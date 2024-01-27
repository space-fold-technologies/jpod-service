#ifndef __DAEMON_DOMAIN_NETWORKING_VIRTUAL_NETWORK_PROVIDER__
#define __DAEMON_DOMAIN_NETWORKING_VIRTUAL_NETWORK_PROVIDER__

#include <vector>
#include <system_error>
#include <memory>
#include <domain/networking/details.h>

namespace spdlog
{
    class logger;
}

struct sockaddr_in;
struct ifreq;
namespace domain::networking
{
    class virtual_network_provider
    {
    public:
        virtual_network_provider();
        virtual ~virtual_network_provider();
        bool has_interface(const std::string &name);
        bool create_interface(const std::string &name, const std::string &alternative, const std::vector<ip_address> &addresses, std::error_code &error);
        bool create_jail_interface(const int jail_id, std::error_code &error);
        bool destroy_interface(const std::string &name, std::error_code &error);

    private:
        int open_socket(const int address_family);
        void convert_address(sockaddr_in *in, size_t size, const std::string &address);
        bool add_interface(const int socket, ifreq &request, const std::string &type, const std::string &name);
        bool rename_interface(const int socket, ifreq &request, const std::string &name);
        bool set_addresses(int socket, const std::string &name, const std::vector<ip_address> &addresses);

    private:
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_NETWORKING_VIRTUAL_NETWORK_PROVIDER__