#include <catch2/catch.hpp>
#include <fakeit.hpp>
#include <domain/networking/address_provider.h>
#include <sole.hpp>

using namespace fakeit;
using namespace domain::networking;

TEST_CASE("ip address provider test case")
{
    GIVEN("an ip v4 cidr")
    {
        std::string ip_v4_cidr("172.23.0.0/16");

        WHEN("no ip address has been requested for")
        {
            std::error_code error{};
            address_provider provider(ip_v4_cidr, "");
            provider.initialize(error);
            THEN("a known a know subnet ip address will be created")
            {
                auto address = provider.fetch_subnet();
                REQUIRE(!error);
                REQUIRE(address->value == "172.23.0.1");
                REQUIRE(address->type == ip_address_type::v4);
                REQUIRE(address->cidr == ip_v4_cidr);
                REQUIRE(address->netmask == "255.255.0.0");
                REQUIRE(address->broadcast == "172.23.255.255");
            }
            THEN("a known ip address will be returned first")
            {
                auto address = provider.fetch_next_available(sole::uuid4().str(), ip_address_type::v4, error);
                REQUIRE(!error);
                REQUIRE(address->value == "172.23.0.2");
                REQUIRE(address->type == ip_address_type::v4);
                REQUIRE(address->cidr == ip_v4_cidr);
                REQUIRE(address->broadcast == "172.23.255.255");
            }
        }

        WHEN("a few ip addresses have been requested")
        {
            address_provider provider(ip_v4_cidr, "");
            std::vector<std::optional<ip_address>> addresses;
            std::error_code error{};
            provider.initialize(error);
            addresses.push_back(provider.fetch_next_available(sole::uuid4().str(), ip_address_type::v4, error));
            addresses.push_back(provider.fetch_next_available(sole::uuid4().str(), ip_address_type::v4, error));
            addresses.push_back(provider.fetch_next_available(sole::uuid4().str(), ip_address_type::v4, error));
            addresses.push_back(provider.fetch_next_available(sole::uuid4().str(), ip_address_type::v4, error));
            THEN("the next ip address in sequence will be returned")
            {

                auto address = provider.fetch_next_available(sole::uuid4().str(), ip_address_type::v4, error);
                REQUIRE(!error);
                REQUIRE(address->value == "172.23.0.6");
                REQUIRE(address->type == ip_address_type::v4);
                REQUIRE(address->cidr == ip_v4_cidr);
            }
        }

        WHEN("a previously requested ip address is returned")
        {
            address_provider provider(ip_v4_cidr, "");
            std::vector<std::optional<ip_address>> addresses;
            std::error_code error{};
            provider.initialize(error);
            addresses.push_back(provider.fetch_next_available(sole::uuid4().str(), ip_address_type::v4, error));
            auto identifier = sole::uuid4().str();
            addresses.push_back(provider.fetch_next_available(identifier, ip_address_type::v4, error));
            addresses.push_back(provider.fetch_next_available(sole::uuid4().str(), ip_address_type::v4, error));
            addresses.push_back(provider.fetch_next_available(sole::uuid4().str(), ip_address_type::v4, error));
            provider.remove(identifier);
            THEN("it will be the next ip address to be given out")
            {
                std::error_code error{};
                auto address = provider.fetch_next_available(sole::uuid4().str(), ip_address_type::v4, error);
                REQUIRE(!error);
                REQUIRE(address->value == "172.23.0.3");
                REQUIRE(address->type == ip_address_type::v4);
                REQUIRE(address->cidr == ip_v4_cidr);
            }
        }
    }
}
