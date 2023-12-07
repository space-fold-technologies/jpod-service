#include <core/networks/addresses/provider.h>
#include <core/networks/addresses/repository.h>
#include <bitset>

namespace core::networks::addresses
{
    AddressProvider::AddressProvider(
        std::shared_ptr<IPAddressRepository> repository,
        const std::string &cidr,
        const std::vector<std::string> &excluded_addresses) : repository(repository),
                                                              cidr(cidr),
                                                              excluded_addresses(excluded_addresses)
    {
    }

    tl::expected<asio::ip::address, std::error_code> AddressProvider::fetch_next_available()
    {
        auto ip_range = this->from_cidr();
        auto used_addresses = repository->fetch_used_addresses();
        used_addresses.insert(used_addresses.end(), excluded_addresses.begin(), excluded_addresses.end());
        auto result = std::find_if_not(ip_range.begin(), ip_range.end(), [&used_addresses](const asio::ip::address_v4 &address) -> bool
                                       { return !(used_addresses.empty() ||
                                                  std::find(used_addresses.begin(), used_addresses.end(), address.to_string()) == used_addresses.end()); });
        if (result != ip_range.end())
        {
            return *result;
        }
        return tl::make_unexpected(std::make_error_code(std::errc::address_not_available));
    }
    asio::ip::address_v4_range AddressProvider::from_cidr()
    {
        auto pos = cidr.find("/");
        auto address = asio::ip::address_v4::from_string(cidr.substr(0, pos));
        int prefix = std::stoi(cidr.substr(pos + 1));
        std::bitset<32> addr_bs(address.to_ulong());
        unsigned long all_ones = ~0;
        std::bitset<32> all_ones_bs(all_ones);
        std::bitset<32> mask_bs = all_ones_bs <<= (32 - prefix);
        std::bitset<32> first_addr_bs = addr_bs & mask_bs;
        std::bitset<32> work_set_bs = mask_bs.flip();
        std::bitset<32> last_addr_bs = first_addr_bs | work_set_bs;
        unsigned long last_addr = last_addr_bs.to_ulong();
        unsigned long first_addr = first_addr_bs.to_ulong();
        auto first_address = asio::ip::address_v4(first_addr);
        auto last_address = asio::ip::address_v4(last_addr);
        return asio::ip::address_v4_range(first_address, last_address);
    }
}