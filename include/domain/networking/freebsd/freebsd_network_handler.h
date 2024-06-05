#ifndef __DAEMON_DOMAIN_NETWORKING_FREEBSD_NETWORK_HANDLER__
#define __DAEMON_DOMAIN_NETWORKING_FREEBSD_NETWORK_HANDLER__

#include <domain/networking/network_handler.h>
#include <memory>

namespace spdlog
{
    class logger;
};

namespace domain::networking
{
    struct interface_order;
    struct network_order;
};

struct sockaddr_in;
struct ifreq;

namespace domain::networking::freebsd
{
    struct address_details
    {
        std::string name;
        std::string ip;
        std::string broadcast;
        std::string netmask;
    };

    struct container_interface_order
    {
        std::string identifier;
        std::string name;
        std::string ip;
        std::string broadcast;
        std::string netmask;
        std::string gateway;
    };

    class freebsd_network_handler : public network_handler
    {
    public:
        freebsd_network_handler();
        virtual ~freebsd_network_handler();
        bool has_bridge(const std::string &name, std::error_code &error) override;
        std::error_code create_bridge(const bridge_order &order) override;
        std::error_code remove_bridge(const std::string &name) override;
        std::error_code bridge_container(const network_order &order, details_callback callback) override;
        std::error_code leave_bridge(const std::string& name, const std::string& first, std::string& second) override;

    private:
        void convert_address(sockaddr_in *in, size_t size, const std::string &address);
        std::error_code add_address(int socket, const address_details &details);
        std::error_code create_pair(int socket, ifreq &first, ifreq &second, const std::string &first_name, const std::string &second_name);
        std::error_code move_interface_to_jail(int socket, ifreq &request, const std::string &container);
        std::error_code activate_interface(int socket, ifreq &request);
        std::error_code send_bridge_order(int socket, const std::string &name, const std::string &member, uint64_t order);
        std::error_code remove_interface(int socket, const std::string &name);
        std::error_code add_interface_to_jail(const container_interface_order& order);
        std::error_code add_route(const std::string &address);

    private:
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_NETWORKING_FREEBSD_NETWORK_HANDLER__