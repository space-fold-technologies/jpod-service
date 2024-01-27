#ifndef __DAEMON_DOMAIN_NETWORKING_ASIO_IP_ADDRESS_PROVIDER__
#define __DAEMON_DOMAIN_NETWORKING_ASIO_IP_ADDRESS_PROVIDER__

#include <domain/networking/address_provider.h>
#include <asio/ip/address.hpp>
#include <asio/ip/address_v4_range.hpp>
#include <asio/ip/address_v6_range.hpp>
#include <memory>
#include <map>

namespace domain::networking
{
    class asio_ip_address_provider : public ip_address_provider
    {
    public:
        asio_ip_address_provider(std::string ip_v4_cidr, std::string ip_v6_cidr);
        virtual ~asio_ip_address_provider();
        void initialize(std::error_code &error) override;
        std::optional<ip_address> fetch_next_available(ip_address_type type, std::error_code &error) override;
        bool remove(const std::string &address_identifier) override;

    private:
        asio::ip::address_v4_range generate_ip_v4_range(std::error_code &error);
        asio::ip::address_v6_range generate_ip_v6_range(std::error_code &error);
        std::optional<asio::ip::address> fetch_next_available_ip_v4(std::error_code &error);
        std::optional<asio::ip::address> fetch_next_available_ip_v6(std::error_code &error);

    private:
        std::string ip_v4_cidr;
        std::string ip_v6_cidr;
        asio::ip::address_v4_range ip_v4_range;
        asio::ip::address_v6_range ip_v6_range;
        std::map<std::string, ip_address> taken_ip_v4_addresses;
        std::map<std::string, ip_address> taken_ip_v6_addresses;
    };
}
#endif // __DAEMON_DOMAIN_NETWORKING_ASIO_IP_ADDRESS_PROVIDER__