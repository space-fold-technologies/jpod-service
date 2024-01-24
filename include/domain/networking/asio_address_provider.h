#ifndef __DAEMON_DOMAIN_NETWORKING_ASIO_IP_ADDRESS_PROVIDER__
#define __DAEMON_DOMAIN_NETWORKING_ASIO_IP_ADDRESS_PROVIDER__

#include <domain/networking/address_provider.h>
#include <asio/ip/address.hpp>
#include <asio/ip/address_v4_range.hpp>
#include <asio/ip/address_v6_range.hpp>
#include <map>

namespace domain::networking
{
    class asio_ip_address_provider : public ip_address_provider
    {
    public:
        asio_ip_address_provider(std::string ip_v4_cidr, std::string ip_v6_cidr);
        virtual ~asio_ip_address_provider();
        ip_address fetch_next_available(ip_address_type type, std::error_code &error) override;
        bool remove(const std::string &address_identifier) override;

    private:
        std::string ip_v4_cidr;
        std::string ip_v6_cidr;
        std::map<std::string, ip_address> taken_addresses;
    };
}
#endif // __DAEMON_DOMAIN_NETWORKING_ASIO_IP_ADDRESS_PROVIDER__