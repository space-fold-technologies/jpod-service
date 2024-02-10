#include <domain/networking/asio_address_provider.h>
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>
#include <algorithm>
#include <range/v3/algorithm/find.hpp>
#include <bitset>
#include <sole.hpp>

namespace rv = ranges::views;

namespace domain::networking
{
    asio_ip_address_provider::asio_ip_address_provider(std::string ip_v4_cidr, std::string ip_v6_cidr) : ip_v4_cidr(std::move(ip_v4_cidr)),
                                                                                                         ip_v6_cidr(std::move(ip_v6_cidr)) {}
    void asio_ip_address_provider::initialize(std::error_code &error)
    {

        if (!ip_v4_cidr.empty())
        {
            ip_v4_range = generate_ip_v4_range(error);
            ip_v4_network = generate_ip_v4_network(ip_v4_cidr, error);
        }
        if (!ip_v6_cidr.empty())
        {
            ip_v6_range = generate_ip_v6_range(error);
            ip_v6_network = generate_ip_v6_network(ip_v6_cidr, error);
        }
    }
    std::optional<ip_address> asio_ip_address_provider::fetch_next_available(ip_address_type type, std::error_code &error)
    {
        if (type == ip_address_type::v4)
        {
            if (auto result = fetch_next_available_ip_v4(error); !error && result.has_value())
            {
                auto identifier = sole::uuid4().str();
                auto address_info = ip_address{
                    identifier,
                    result->to_string(),
                    ip_v4_network.netmask().to_string(),
                    ip_v4_network.broadcast().to_string(),
                    ip_address_type::v4,
                    ip_v4_cidr};
                taken_ip_v4_addresses.emplace(identifier, address_info);
                return taken_ip_v4_addresses.at(identifier);
            }
            else if (type == ip_address_type::v6)
            {
                if (auto result = fetch_next_available_ip_v6(error); !error && result.has_value())
                {
                    auto identifier = sole::uuid4().str();
                    auto address_info = ip_address{
                        identifier,
                        result->to_string(),
                        "",// ip_v6_network.netmask().to_string(),
                        "",//ip_v6_network.broadcast().to_string(),
                        ip_address_type::v6,
                        ip_v6_cidr};
                    taken_ip_v6_addresses.emplace(identifier, address_info);
                    return taken_ip_v6_addresses.at(identifier);
                }
            }
        }
        return std::nullopt;
    }
    std::optional<asio::ip::address> asio_ip_address_provider::fetch_next_available_ip_v4(std::error_code &error)
    {
        auto map_to_ip = [](const auto &entry) -> std::string
        { return entry.second.value; };
        auto take_ip_addresses = taken_ip_v4_addresses | rv::transform(map_to_ip) | ranges::to<std::vector<std::string>>();

        auto first_free_ip = [&take_ip_addresses](const auto &ip) -> bool
        { return !(take_ip_addresses.empty() || ranges::find(take_ip_addresses, ip.to_string()) == take_ip_addresses.end()); };

        auto result = std::find_if_not(ip_v4_range.begin(), ip_v4_range.end(), first_free_ip);
        if (result == ip_v4_range.end())
        {
            return std::nullopt;
        }
        return std::make_optional(*result);
    }
    std::optional<asio::ip::address> asio_ip_address_provider::fetch_next_available_ip_v6(std::error_code &error)
    {
        auto map_to_ip = [](const auto &entry) -> std::string
        { return entry.second.value; };
        auto take_ip_addresses = taken_ip_v6_addresses | rv::transform(map_to_ip) | ranges::to<std::vector<std::string>>();

        auto first_free_ip = [&take_ip_addresses](const auto &ip) -> bool
        { return !(take_ip_addresses.empty() || ranges::find(take_ip_addresses, ip.to_string()) == take_ip_addresses.end()); };

        auto result = std::find_if_not(ip_v6_range.begin(), ip_v6_range.end(), first_free_ip);
        if (result == ip_v6_range.end())
        {
            return std::nullopt;
        }
        return std::make_optional(*result);
    }
    bool asio_ip_address_provider::remove(const std::string &address_identifier)
    {
        if (auto position = taken_ip_v4_addresses.find(address_identifier); position != taken_ip_v4_addresses.end())
        {
        }
        else if (auto position = taken_ip_v6_addresses.find(address_identifier); position != taken_ip_v6_addresses.end())
        {
        }
        return false;
    }
    asio::ip::address_v4_range asio_ip_address_provider::generate_ip_v4_range(std::error_code &error)
    {
        auto pos = ip_v4_cidr.find("/");
        auto address = asio::ip::address_v4::from_string(ip_v4_cidr.substr(0, pos));
        int prefix = std::stoi(ip_v4_cidr.substr(pos + 1));
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

    asio::ip::network_v4 asio_ip_address_provider::generate_ip_v4_network(const std::string &cidr, std::error_code &error)
    {
        return asio::ip::make_network_v4(cidr, error);
    }
    asio::ip::network_v6 asio_ip_address_provider::generate_ip_v6_network(const std::string &cidr, std::error_code &error)
    {
        auto pos = cidr.find("/");
        auto address = asio::ip::address_v6::from_string(cidr.substr(0, pos), error);
        std::size_t size = std::stoul(cidr.substr(pos + 1));
        return asio::ip::make_network_v6(address, size);
    }

    asio::ip::address_v6_range asio_ip_address_provider::generate_ip_v6_range(std::error_code &error)
    {
        auto pos = ip_v6_cidr.find("/");
        auto start_address = asio::ip::address_v6::from_string(ip_v6_cidr.substr(0, pos));
        std::size_t size = std::stoul(ip_v6_cidr.substr(pos + 1));
        auto bytes = start_address.to_bytes();
        std::size_t offset = size >> 3;
        uint8_t to_add = 1 << (8 - (size & 0x7));
        while (to_add)
        {
            int value = bytes[offset] + to_add;
            bytes[offset] = value & 0xFF;
            to_add = value >> 8;
            if (offset == 0)
            {
                break;
            }
            offset--;
        }
        auto end_address = asio::ip::address_v6(bytes);
        return asio::ip::address_v6_range(start_address, end_address);
    }
    asio_ip_address_provider::~asio_ip_address_provider()
    {
    }
}