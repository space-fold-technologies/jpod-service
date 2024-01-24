#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <domain/networking/asio_address_provider.h>

using namespace fakeit;
using namespace domain::networking;

TEST_CASE("ip address provider test case")
{
    GIVEN("an ip v4 cidr")
    {
        std::string ip_v4_cidr("172.23.0.1/16");

        WHEN("no ip address has been requested for")
        {
            asio_ip_address_provider provider(ip_v4_cidr, "");

            THEN("a known ip address will be returned first")
            {
                std::error_code error{};
                auto address = provider.fetch_next_available(ip_address_type::v4, error);
                REQUIRE(!error);
                REQUIRE(address.value == "172.23.0.2");
                REQUIRE(address.type == ip_address_type::v4);
                REQUIRE(address.cidr == ip_v4_cidr);
            }
        }

        WHEN("a few ip addresses have been requested")
        {
            asio_ip_address_provider provider(ip_v4_cidr, "");
            std::vector<ip_address> addresses;
            addresses.push_back(provider.fetch_next_available(ip_address_type::v4, error));
            addresses.push_back(provider.fetch_next_available(ip_address_type::v4, error));
            addresses.push_back(provider.fetch_next_available(ip_address_type::v4, error));
            addresses.push_back(provider.fetch_next_available(ip_address_type::v4, error));
            THEN("the next ip address in sequence will be returned")
            {
                std::error_code error{};
                auto address = provider.fetch_next_available(ip_address_type::v4, error);
                REQUIRE(!error);
                REQUIRE(address.value == "172.23.0.6");
                REQUIRE(address.type == ip_address_type::v4);
                REQUIRE(address.cidr == ip_v4_cidr);
            }
        }

        WHEN("a previously requested ip address is returned")
        {
            asio_ip_address_provider provider(ip_v4_cidr, "");
            std::vector<ip_address> addresses;
            addresses.push_back(provider.fetch_next_available(ip_address_type::v4, error));
            addresses.push_back(provider.fetch_next_available(ip_address_type::v4, error));
            addresses.push_back(provider.fetch_next_available(ip_address_type::v4, error));
            addresses.push_back(provider.fetch_next_available(ip_address_type::v4, error));
            provider.remove(addresses.at(1).identifier);
            THEN("it will be the next ip address to be given out")
            {
                std::error_code error{};
                auto address = provider.fetch_next_available(ip_address_type::v4, error);
                REQUIRE(!error);
                REQUIRE(address.value == "172.23.0.3");
                REQUIRE(address.type == ip_address_type::v4);
                REQUIRE(address.cidr == ip_v4_cidr);
            }
        }
    }
}
