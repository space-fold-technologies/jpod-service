#include <core/databases/connection.h>
#include <core/databases/statement.h>
#include <core/databases/result_set.h>
#include <spdlog/spdlog.h>
#include <sqlite3.h>

namespace core::databases
{
    Connection::Connection(const std::string &path) : path(path), logger(spdlog::get("jpod"))
    {
    }
    void Connection::open()
    {
        sqlite3 *db;
        logger->info("DATABASE FILE {}", path);
        if (sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, nullptr) != SQLITE_OK)
        {
            logger->error("SQLITE 3 ERROR ON OPENING {}", sqlite3_errmsg(db));
            sqlite3_close_v2(db);
        }
        else
        {

            sqlite3_trace_v2(
                db, SQLITE_TRACE_STMT | SQLITE_TRACE_PROFILE,
                [](unsigned flags, void *context, void *p, void *x) -> int
                { return 0; },
                this);
        }
        this->instance = database_ptr<sqlite3>(db, [](sqlite3 *db)
                                               { sqlite3_close_v2(db); });
    }
    bool Connection::is_ok()
    {
        return sqlite3_db_readonly(instance.get(), nullptr);
    }
    bool Connection::has_table(const std::string &name)
    {
        Statement statement(this, "SELECT count(*) AS exist FROM sqlite_master WHERE type='table' AND name=?");
        statement.bind(1, name);
        auto resultset = statement.execute_query();
        return (1 == resultset.fetch<int>("exist"));
    }
    int Connection::execute(const std::string &query)
    {
        char *sql_error_message;
        if (sqlite3_exec(instance.get(), query.c_str(), nullptr, nullptr, &sql_error_message) != SQLITE_OK)
        {
            logger->error("CONNECTION EXECUTION ERROR : {}", sql_error_message);
            sqlite3_free(sql_error_message);
            return -1;
        }
        return sqlite3_changes(instance.get());
    }
    Connection::~Connection()
    {
        instance.reset();
    }
}