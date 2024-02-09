#ifndef __JPOD_CONTAINERS_TESTING_HELPERS__
#define __JPOD_CONTAINERS_TESTING_HELPERS__

#include <cmrc/cmrc.hpp>
#include <core/sql/data_source.h>
#include <core/sql/migrations.h>
#include <optional>
#include <domain/images/sql_repository.h>
#include <sole.hpp>
#include <spdlog/spdlog.h>

CMRC_DECLARE(resources);

namespace core::sql
{
    // Will need to find a way to start and stop migrations
    inline auto migrate(core::sql::pool::data_source &data_source, const std::string &path) -> void
    {
        migration_handler handler(data_source, path);
        handler.migrate();
    }
    inline auto create_test_image(
        core::sql::pool::data_source &data_source,
        const std::string &registry,
        const std::string &name,
        const std::string &tag) -> std::optional<std::string>
    {
        auto logger = spdlog::get("jpod");
        domain::images::sql_image_repository repository(data_source);
        domain::images::registry_details details{"venka", "https://hub.venka.com/images", registry};
        if (std::error_code error = repository.add_registry(details); error)
        {
            logger->error("ERR: {}", error.message());
        }
        else
        {
            std::string identifier = sole::uuid4().str();
            domain::images::image_details details{
                identifier,
                name,
                tag,
                "linux",
                "busybox",
                "3.18.4",
                5 * 1000 * 1024,
                "",
                "hub.venka.com",
                std::map<std::string, std::string>{{"EMAIL", "dev.survivor.com"}},
                std::map<std::string, std::string>{{"mount.devfs", ""}},
                std::map<std::string, std::string>{{"JAVA_HOME", "/opt/java"}},
                std::vector<domain::images::mount_point>{
                    domain::images::mount_point{"devfs", "/dev", "rw", 0},
                    domain::images::mount_point{"linprocfs", "/dev/shm", "rw", 0}}};
            if (error = repository.save_image_details(details); error)
            {
                logger->error("ERR: {}", error.message());
                return "";
            }
            return identifier;
        }
        return std::nullopt;
    }
}

#endif // __JPOD_CONTAINERS_TESTING_HELPERS__