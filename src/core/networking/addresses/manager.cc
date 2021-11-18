#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/network_v4.hpp>
#include <boost/lexical_cast.hpp>
#include <core/networking/addresses/manager.h>
#include <stdio.h>
#include <spdlog/spdlog.h>
#include <definitions.h>

namespace networking::addresses
{

AddressManager::AddressManager (networking::repository::NetworkStore &store, 
                                const std::string &cidr, 
                                const std::vector<std::string>& pre_excluded_ip_address ) :          
store ( store ),
cidr ( cidr ),
pre_excluded_ip_address ( pre_excluded_ip_address )
{
}
std::optional<boost::asio::ip::address> AddressManager::fetch_next_available()
{
    auto ip_range = this->from_cidr ( cidr );
    auto used_addresses = store.fetch_used_addresses();
    used_addresses.insert(used_addresses.begin(), pre_excluded_ip_address.begin(), pre_excluded_ip_address.end());
    auto result = std::find_if_not ( ip_range.begin(), ip_range.end(), [&used_addresses] ( const address_v4& address )->bool{
        return ! ( used_addresses.empty() ||
                   std::find ( used_addresses.begin(), used_addresses.end(), address.to_string() ) == used_addresses.end() );} );
    return std::make_optional ( *result );
}

address_v4_range AddressManager::from_cidr ( const std::string &cidr_address )
{
    auto pos = cidr_address.find ( "/" );
    auto address = boost::asio::ip::address_v4::from_string ( cidr_address.substr ( 0, pos ) );
    int prefix = boost::lexical_cast<int> ( cidr_address.substr ( pos + 1 ) );
    std::bitset<32> addr_bs ( address.to_ulong() );
    unsigned long all_ones =~ 0;
    std::bitset<32> all_ones_bs ( all_ones );
    std::bitset<32> mask_bs = all_ones_bs<<= ( 32-prefix );
    std::bitset<32> first_addr_bs = addr_bs & mask_bs;
    std::bitset<32> work_set_bs = mask_bs.flip();
    std::bitset<32> last_addr_bs = first_addr_bs | work_set_bs;
    unsigned long last_addr = last_addr_bs.to_ulong();
    unsigned long first_addr = first_addr_bs.to_ulong();
    auto first_address = boost::asio::ip::address_v4 ( first_addr );
    auto last_address = boost::asio::ip::address_v4 ( last_addr );
    return address_v4_range ( first_address, last_address );
}

//} // namespace networking::addresses
}// namespace networking::addresses
