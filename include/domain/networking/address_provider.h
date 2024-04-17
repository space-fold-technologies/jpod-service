#ifndef __DAEMON_DOMAIN_NETWORKING_ASIO_ADDRESS_PROVIDER__
#define __DAEMON_DOMAIN_NETWORKING_ASIO_ADDRESS_PROVIDER__

#include <asio/ip/address.hpp>
#include <asio/ip/address_v4_range.hpp>
#include <asio/ip/address_v6_range.hpp>
#include <asio/ip/network_v4.hpp>
#include <asio/ip/network_v6.hpp>
#include <domain/networking/details.h>
#include <memory>
#include <map>

namespace domain::networking
{
    class address_provider
    {
    public:
        address_provider(std::string ip_v4_cidr, std::string ip_v6_cidr);
        virtual ~address_provider();
        void initialize(std::error_code &error);
        std::optional<ip_address> fetch_subnet();
        std::optional<ip_address> fetch_next_available(const std::string &identifier, ip_address_type type, std::error_code &error);
        bool remove(const std::string &address_identifier);
        std::size_t total_taken(ip_address_type type);

    private:
        asio::ip::address_v4_range generate_ip_v4_range(std::error_code &error);
        asio::ip::address_v6_range generate_ip_v6_range(std::error_code &error);
        asio::ip::network_v4 generate_ip_v4_network(const std::string &cidr, std::error_code &error);
        asio::ip::network_v6 generate_ip_v6_network(const std::string &cidr, std::error_code &error);
        std::optional<asio::ip::address> fetch_next_available_ip_v4(std::error_code &error);
        std::optional<asio::ip::address> fetch_next_available_ip_v6(std::error_code &error);

    private:
        std::string ip_v4_cidr;
        std::string ip_v6_cidr;
        asio::ip::address_v4_range ip_v4_range;
        asio::ip::address_v6_range ip_v6_range;
        asio::ip::network_v4 ip_v4_network;
        asio::ip::network_v6 ip_v6_network;
        std::map<std::string, ip_address> taken_ip_v4_addresses;
        std::map<std::string, ip_address> taken_ip_v6_addresses;
    };
}
#endif // __DAEMON_DOMAIN_NETWORKING_ASIO_ADDRESS_PROVIDER__
