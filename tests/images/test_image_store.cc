#include <catch2/catch.hpp>
#include "helpers.h"
#include <domain/images/sql_repository.h>
#include <sole.hpp>
#include <chrono>
#include <fmt/format.h>

TEST_CASE("image repository case")
{
    core::sql::pool::data_source data_source(":memory:", 2);
    data_source.initialize();
    core::sql::migrate(data_source, "migrations");
    domain::images::sql_image_repository repository(data_source);
    SECTION("saving registry details")
    {

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
    SECTION("saving image details")
    {
        std::string identifier = sole::uuid4().str();
        domain::images::image_details details{
            identifier,
            "alpine",
            "latest",
            "linux",
            "busybox",
            "3.18.4",
            5 * 1000 * 1024,
            "",
            "hub.venka.com",
            std::map<std::string, std::string>{{"EMAIL" , "dev.survivor.com"}},
            std::map<std::string, std::string>{{"mount.devfs", ""}},
            std::map<std::string, std::string>{{"JAVA_HOME", "/opt/java"}},
            std::vector<domain::images::mount_point>{
                domain::images::mount_point{"devfs", "/dev", "rw", 0},
                domain::images::mount_point{"linprocfs", "/dev/shm", "rw", 0}}};
        std::error_code error = repository.save_image_details(details);
        REQUIRE_FALSE(error);

        SECTION("can fetch image details")
        {
            REQUIRE(repository.has_image("hub.venka.com", "alpine", "latest"));
            auto result = repository.fetch_image_details("hub.venka.com", "alpine", "latest");
            REQUIRE(result.has_value());
        }
        SECTION("can fetch image identifier")
        {
            auto result = repository.fetch_image_identifier("hub.venka.com", "alpine", "latest");
            REQUIRE(result.has_value());
            REQUIRE(identifier == *result);
        }
        SECTION("can fetch only mount points")
        {
            auto mount_points = repository.fetch_image_mount_points("hub.venka.com", "alpine", "latest");
            REQUIRE(mount_points.size() == 2);
        }
        SECTION("can search for the image summaries with partial query")
        {
            auto summaries = repository.fetch_matching_details("alp");
            REQUIRE(summaries.size() == 1);
            auto summary = summaries.at(0);
            REQUIRE(summary.identifier == identifier);
            REQUIRE(summary.name == details.name);
            REQUIRE(summary.tag == details.tag);
            REQUIRE(summary.repository == "venka");
            REQUIRE(summary.size == details.size);
            REQUIRE(summary.created_at < std::chrono::system_clock::now());
        }
        SECTION("can return all image summaries with no query")
        {
            auto summaries = repository.fetch_matching_details("");
            REQUIRE(summaries.size() == 1);
            auto summary = summaries.at(0);
            REQUIRE(summary.identifier == identifier);
            REQUIRE(summary.name == details.name);
            REQUIRE(summary.tag == details.tag);
            REQUIRE(summary.repository == "venka");
            REQUIRE(summary.size == details.size);
            REQUIRE(summary.created_at < std::chrono::system_clock::now());
        }
        SECTION("can remove an existing image entry")
        {
            std::error_code error = repository.remove(identifier);
            REQUIRE_FALSE(error);
            REQUIRE_FALSE(repository.has_image("hub.venka.com", "alpine", "latest"));
        }
    }
}