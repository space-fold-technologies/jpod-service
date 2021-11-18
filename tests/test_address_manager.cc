#include <boost/asio/ip/address.hpp>
#include <catch.hpp>
#include <core/networking/addresses/manager.h>
#include <core/networking/repository/store.h>
#include <fakeit.hpp>

TEST_CASE("IP Address Manager") {
  using namespace networking::addresses;
  using namespace networking::repository;
  using boost::asio::ip::address;
  using namespace fakeit;
  Mock<networking::repository::NetworkStore> mock;
 std::vector<std::string> excluded_ip_addresses = {"172.23.0.0","172.23.0.1"};
  GIVEN("CIDR") {
    AddressManager manager(mock.get(), "172.23.0.1/16", excluded_ip_addresses);
    WHEN("No IP containers exist") {
      When(Method(mock, fetch_used_addresses)).Return({});
      THEN("The first IP address can be acquired from the sequence") {
        auto ip_address = manager.fetch_next_available();
        REQUIRE(ip_address.has_value());
        REQUIRE(ip_address->is_v4());
        REQUIRE(ip_address->to_string() == "172.23.0.2");
      }
    }
    WHEN("A few container IP addresses have been given out") {
      When(Method(mock, fetch_used_addresses)).Return({"172.23.0.2", "172.23.0.3", "172.23.0.4", "172.23.0.5"});
      THEN("The largest IP address in the sequence is given out") {
        auto ip_address = manager.fetch_next_available();
        REQUIRE(ip_address.has_value());
        REQUIRE(ip_address->is_v4());
        REQUIRE(ip_address->to_string() == "172.23.0.6");
      }
    }

    WHEN("An IP address in a sequence is removed") {
      When(Method(mock, fetch_used_addresses)).Return({"172.23.0.2", "172.23.0.4", "172.23.0.5", "172.23.0.6"});
      THEN("The IP address missing in the sequence will be taken up") {
        auto ip_address = manager.fetch_next_available();
        REQUIRE(ip_address.has_value());
        REQUIRE(ip_address->is_v4());
        REQUIRE(ip_address->to_string() == "172.23.0.3");
      }
    }
  }
}
