#include <core/sql/migrations.h>
#include <core/sql/data_source.h>
#include <core/sql/connection_proxy.h>
#include <core/sql/statement.h>
#include <core/sql/result_set.h>
#include <core/sql/transaction.h>
#include <core/sql/error.h>
#include <spdlog/spdlog.h>
#include <cmrc/cmrc.hpp>
#include <fmt/format.h>

CMRC_DECLARE(resources);

namespace core::sql
{
    migration_handler::migration_handler(pool::data_source &data_source, std::string path) : data_source(data_source),
                                                                                             path(std::move(path)),
                                                                                             migrations{},
                                                                                             logger(spdlog::get("jpod"))
    {
    }

    void migration_handler::migrate()
    {
        if (!has_table())
        {
            if (!create_table())
            {
                logger->error("failed to create migration table");
                return;
            }
        }
        auto fs = cmrc::resources::get_filesystem();
        for (const auto &entry : fs.iterate_directory(path))
        {
            if (entry.is_file() && entry.filename().find(".sql") != std::string::npos)
            {
                auto file = fs.open(fmt::format("{}/{}", path, entry.filename()));
                std::string content(file.begin(), file.end());
                if (auto error = apply(content, entry.filename()); error)
                {
                    logger->error("failed to perform migration: {}", error.message());
                    return;
                }
            }
        }
    }
    bool migration_handler::has_been_done(connection_proxy &proxy, const std::string &migration)
    {
        std::string sql("SELECT "
                        "COUNT(*) AS has_migration "
                        "FROM migration_record_tb AS m "
                        "WHERE m.migration = ?");
        auto statement = proxy->statement(sql);
        statement.bind(1, migration);
        if (auto result = statement.execute_query(); !result.has_next())
        {
            return false;
        }
        else
        {
            return result.fetch<bool>("has_migration");
        }
    }
    std::error_code migration_handler::register_migration(connection_proxy &proxy, const std::string &migration)
    {
        std::string sql("INSERT INTO migration_record_tb(migration, dirty) VALUES(?, ?)");
        core::sql::transaction txn(proxy);
        auto statement = proxy->statement(sql);
        statement.bind(1, migration);
        statement.bind(2, false);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    std::error_code migration_handler::register_state(connection_proxy &proxy, const std::string &migration, bool state)
    {
        std::string sql("UPDATE "
                        "migration_record_tb "
                        "SET "
                        "dirty = ? "
                        "WHERE "
                        "migration = ?");
        core::sql::transaction txn(proxy);
        auto statement = proxy->statement(sql);
        statement.bind(1, state);
        statement.bind(2, migration);
        if (auto result_code = statement.execute(); result_code < 0)
        {
            return core::sql::errors::make_error_code(result_code);
        }
        txn.commit();
        return {};
    }
    std::error_code migration_handler::apply(const std::string &content, const std::string &migration)
    {

        if (auto proxy = data_source.connection(); proxy.is_valid())
        {
            if (has_been_done(proxy, migration))
            {
                return {};
            }
            else if (auto error = register_migration(proxy, migration); error)
            {
                return error;
            }
            else if (proxy->execute(content) < 0)
            {
                logger->error("migration {} failed", migration);
                return register_state(proxy, migration, true);
                return std::make_error_code(std::errc::operation_canceled);
            }
            logger->info("migration: {} added", migration);
            return register_state(proxy, migration, false);
            return {};
        }
        else
        {
            logger->info("failed to open connection to migrate: {}", migration);
            return {};
        }
    }
    bool migration_handler::create_table()
    {
        std::string sql("CREATE TABLE IF NOT EXISTS migration_record_tb("
                        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        " migration TEXT NOT NULL,"
                        " dirty BOOLEAN "
                        ")");
        if (auto proxy = data_source.connection(); proxy.is_valid())
        {
            if (proxy->execute(sql) < 0)
            {
                logger->error("BASE TABLE {} FAILED", sql);
                return false;
            }
            return true;
        }
        return false;
    }
    bool migration_handler::has_table()
    {
        std::string sql("SELECT "
                        "COUNT(*) AS has_migrations_table "
                        "FROM sqlite_master AS c "
                        "WHERE type = ? AND name = ?");
        auto connection = data_source.connection();
        auto statement = connection->statement(sql);
        statement.bind(1, "table");
        statement.bind(2, "migration_record_tb");
        if (auto result = statement.execute_query(); !result.has_next())
        {
            return false;
        }
        else
        {
            return result.fetch<bool>("has_migrations_table");
        }
    }
    migration_handler::~migration_handler()
    {
    }
}
