#include <catch2/catch.hpp>
#include "helpers.h"
#include <domain/images/sql_repository.h>
#include <domain/containers/sql_repository.h>
#include <sole.hpp>
#include <chrono>
#include <fmt/format.h>

TEST_CASE("container repository case")
{
    core::sql::pool::data_source data_source(":memory:", 2);
    data_source.initialize();
    core::sql::migrate(data_source, "migrations");
    auto image_identifier = core::sql::create_test_image(data_source, "hub.venka.com", "alpine", "latest");
    std::string container_identifier = sole::uuid4().str();
    domain::containers::sql_container_repository repository(data_source);
    SECTION("can fetch image details")
    {
        auto result = repository.fetch_image_details("hub.venka.com", "alpine", "latest");
        REQUIRE(result.has_value());
    }
    SECTION("can save container details")
    {
        domain::containers::container_properties properties{
            container_identifier,
            "kepler",
            *image_identifier,
            std::map<std::string, std::string>{},
            std::map<std::string, std::string>{},
            std::map<std::string, std::string>{},
            "/bin/sh",
            "nats"};
        std::error_code error = repository.save(properties);
        REQUIRE_FALSE(error);
        SECTION("can fetch container details")
        {
            auto result = repository.fetch(container_identifier);
            REQUIRE(result.has_value());
            REQUIRE(result->identifier == container_identifier);
            REQUIRE(result->entry_point == "/bin/sh");
            REQUIRE(result->network_properties == "nats");
            result = repository.first_match("kepler");
            REQUIRE(result.has_value());
            REQUIRE(result->identifier == container_identifier);
            REQUIRE(result->entry_point == "/bin/sh");
            REQUIRE(result->network_properties == "nats");
        }
        SECTION("can fetch identifier")
        {
            auto result = repository.first_identifier_match("kepler");
            REQUIRE(result.has_value());
            REQUIRE(*result == container_identifier);
        }
        SECTION("can fetch container summaries")
        {
            auto summaries = repository.fetch_match("kep", "all");
            REQUIRE(summaries.size() == 1);

        }
        // Need to write a status update for container repositoy
    }
}