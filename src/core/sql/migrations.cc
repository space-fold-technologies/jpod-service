#include <core/sql/migrations.h>
#include <core/sql/data_source.h>
#include <core/sql/connection_proxy.h>
#include <spdlog/spdlog.h>
#include <cmrc/cmrc.hpp>
#include <fmt/format.h>

CMRC_DECLARE(resources);

namespace core::sql
{
    migration_handler::migration_handler(pool::data_source &data_source, std::string path) : data_source(data_source),
                                                                                             path(std::move(path)),
                                                                                             logger(spdlog::get("jpod"))
    {
    }

    void migration_handler::migrate()
    {
        auto fs = cmrc::resources::get_filesystem();
        for (const auto &entry : fs.iterate_directory(path))
        {
            if (entry.is_file() && entry.filename().find(".sql") != std::string::npos)
            {
                auto file = fs.open(fmt::format("{}/{}", path, entry.filename()));
                std::string content(file.begin(), file.end());
                apply(content, entry.filename());
            }
        }
    }
    void migration_handler::apply(const std::string &content, const std::string &migration)
    {
        logger->info("adding script: {}\n{}", migration, content);
        if (auto proxy = data_source.connection(); proxy.is_valid())
        {
            if (proxy->execute(content) < 0)
            {
                logger->error("MIGRATION {} FAILED", migration);
            }
        }
    }
    migration_handler::~migration_handler()
    {
    }
}
