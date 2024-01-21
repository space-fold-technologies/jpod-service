#include <core/sql/migrations.h>
#include <core/sql/data_source.h>
#include <core/sql/connection_proxy.h>
#include <algorithm>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <sstream>
#include <fstream>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/for_each.hpp>

namespace fs = std::filesystem;

namespace core::sql
{
    migration_handler::migration_handler(pool::data_source &data_source, const std::string &folder) : data_source(data_source),
                                                                                                      folder(folder),
                                                                                                      logger(spdlog::get("jpod"))
    {
    }
    void migration_handler::migrate()
    {

        auto file_iterator = fs::directory_iterator(folder);
        auto migration_script_check = [](const std::filesystem::directory_entry &file) -> bool
        { return file.is_regular_file() && file.path().extension() == ".sql"; };
        auto migration_files = ranges::find_if(file_iterator, migration_script_check);

        ranges::for_each(
            migration_files,
            [&](const fs::directory_entry &file)
            {
                logger->info("MIGRATION {}", file.path().filename().string());
                std::ifstream in(file.path(), std::ios::in | std::ios::binary);
                std::ostringstream content;
                content << in.rdbuf();
                in.close();

                apply(content.str(), file.path().filename().string());
            });
    }
    void migration_handler::apply(const std::string &content, const std::string &migration)
    {
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
