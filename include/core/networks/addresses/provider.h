#ifndef __JPOD_SERVICE_CORE_NETWORKS_ADDRESSES_PROVIDER__
#define __JPOD_SERVICE_CORE_NETWORKS_ADDRESSES_PROVIDER__
#include <asio/ip/address.hpp>
#include <asio/ip/address_v4_range.hpp>
#include <memory>
#include <system_error>
#include <string>
#include <vector>
#include <tl/expected.hpp>

namespace core::networks::addresses
{
    class IPAddressRepository;
    class AddressProvider
    {
    public:
        AddressProvider(
            std::shared_ptr<IPAddressRepository> repository,
            const std::string &cidr,
            const std::vector<std::string> &excluded_addresses);
        virtual ~AddressProvider() = default;
        tl::expected<asio::ip::address, std::error_code> fetch_next_available();

    private:
        asio::ip::address_v4_range from_cidr();

    private:
        std::shared_ptr<IPAddressRepository> repository;
        const std::string &cidr;
        const std::vector<std::string> &excluded_addresses;
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_ADDRESSES_PROVIDER__