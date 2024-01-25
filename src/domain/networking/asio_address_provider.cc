#include <domain/networking/asio_address_provider.h>
#include <asio/ip/address.hpp>
#include <asio/ip/address_v4_range.hpp>
#include <asio/ip/address_v6_range.hpp>
#include <bitset>
#include <sole.hpp>

namespace domain::networking
{
    asio_ip_address_provider::asio_ip_address_provider(std::string ip_v4_cidr, std::string ip_v6_cidr) : ip_v4_cidr(std::move(ip_v4_cidr)),
                                                                                                         ip_v6_cidr(std::move(ip_v6_cidr)),
                                                                                                         ip_v4_range(nullptr),
                                                                                                         ip_v6_range(nullptr) {}
    void asio_ip_address_provider::initialize(std::error_code &error)
    {

        if (!ip_v4_cidr.empty())
        {
            ip_v4_range = generate_ip_v4_range(error);
        }
        if (!ip_v6_cidr.empty())
        {
            ip_v6_range = generate_ip_v6_range(error);
        }
    }
    ip_address asio_ip_address_provider::fetch_next_available(ip_address_type type, std::error_code &error)
    {
        if (type == ip_address_type::v4)
        {
            if (auto result = fetch_next_available_ip_v4(error); !error && result.has_value())
            {
                auto identifier = sole::uuid4().str();
                return ip_address{identifier, result->to_string(), ip_address_type::v4, ip_v4_cidr};
            }
        }
        else if (type == ip_address_type::v6)
        {
            if (auto result = fetch_next_available_ip_v6(error); !error && result.has_value())
            {
                auto identifier = sole::uuid4().str();
                return ip_address{identifier, result->to_string(), ip_address_type::v6, ip_v6_cidr};
            }
        }
    }
    std::optional<asio::ip::address> asio_ip_address_provider::fetch_next_available_ip_v4(std::error_code &error)
    {
        return std::nullopt;
    }
    std::optional<asio::ip::address> asio_ip_address_provider::fetch_next_available_ip_v6(std::error_code &error)
    {
        return std::nullopt;
    }
    bool asio_ip_address_provider::remove(const std::string &address_identifier) {}
    std::unique_ptr<asio::ip::address_v4_range> asio_ip_address_provider::generate_ip_v4_range(std::error_code &error)
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
        return std::make_unique<asio::ip::address_v4_range>(first_address, last_address);
    }
    std::unique_ptr<asio::ip::address_v6_range> asio_ip_address_provider::generate_ip_v6_range(std::error_code &error)
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
        return std::make_unique<asio::ip::address_v6_range>(start_address, end_address);
    }
    asio_ip_address_provider::~asio_ip_address_provider()
    {
    }
}