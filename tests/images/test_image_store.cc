#include <catch2/catch.hpp>
#include "helpers.h"
#include <domain/images/sql_repository.h>
#include <sole.hpp>
#include <fmt/format.h>

TEST_CASE("repository instruction case")
{
    core::sql::pool::data_source data_source(":memory:", 2);
    data_source.initialize();
    core::sql::migrate(data_source, "migrations");
    SECTION("saving registry details")
    {
        domain::images::sql_image_repository repository(data_source);
        domain::images::registry_details details{"venka", "https://hub.venka.com/images", "hub.venka.com"};
        std::error_code error = repository.add_registry(details);
        REQUIRE_FALSE(error);
        SECTION("fetching registry details by path")
        {
            auto result = repository.fetch_registry_by_path("hub.venka.com");
            REQUIRE(result.has_value());
            REQUIRE(result->uri == "https://hub.venka.com/images");
            REQUIRE(result->token == "");
        }
        SECTION("fetching registry details by name")
        {
            auto result = repository.fetch_registry_by_name("venka");
            REQUIRE(result.has_value());
            REQUIRE(result->uri == "https://hub.venka.com/images");
            REQUIRE(result->token == "");
        }
        SECTION("updating authorization details")
        {
             domain::images::authorization_update update{"hub.venka.com", "*$)_@_)-492-94--22759302"};
            std::error_code error = repository.update_token(update);
            REQUIRE_FALSE(error);
            SECTION("can token in registry details")
            {
                auto result = repository.fetch_registry_by_name("venka");
                REQUIRE(result.has_value());
                REQUIRE(result->uri == "https://hub.venka.com/images");
                REQUIRE(result->token == "*$)_@_)-492-94--22759302");
                result = repository.fetch_registry_by_path("hub.venka.com");
                REQUIRE(result.has_value());
                REQUIRE(result->uri == "https://hub.venka.com/images");
                REQUIRE(result->token == "*$)_@_)-492-94--22759302");
            }
        }
    }
}