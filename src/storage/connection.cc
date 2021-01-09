#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <storage/connection.h>
#include <storage/resultset.h>
#include <storage/statement.h>

namespace database {
  Connection::Connection(const std::string &path) : path(path) {
  }
  sqlite3 *database::Connection::handle() {
    return db_instance.get();
  }

  void Connection::open() {
    sqlite3 *db;
    spdlog::get("app")->info("DATABASE FILE {}", path);
    if (sqlite3_open_v2(path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX, nullptr) != SQLITE_OK) {
      spdlog::get("app")->error("SQLITE 3 ERROR ON OPENING {}", sqlite3_errmsg(db));
      sqlite3_close_v2(db);
    } else {

      sqlite3_trace_v2(
          db, SQLITE_TRACE_STMT | SQLITE_TRACE_PROFILE, [](unsigned flags, void *context, void *p, void *x) -> int {
            spdlog::get("app")->debug("P: {} :X {}", p, x);
            return 0;
          },
          this);
    }
    this->db_instance = db_ptr<sqlite3>(db, [](sqlite3 *db) {
      sqlite3_close_v2(db);
      spdlog::get("app")->info("CLOSED DATABASE");
    });
  }

  bool Connection::isOK() {
    return sqlite3_db_readonly(db_instance.get(), nullptr);
  }

  bool Connection::hasTable(const std::string &name) {
    Statement statement(this, "SELECT count(*) AS exist FROM sqlite_master WHERE type='table' AND name=?");
    statement.bind(1, name);
    auto resultset = statement.executeQuery();
    return (1 == resultset.fetch<int>("exist"));
  }

  int Connection::execute(const std::string &query) {
    char *sql_error_message;
    if (sqlite3_exec(db_instance.get(), query.c_str(), nullptr, nullptr, &sql_error_message) != SQLITE_OK) {
      spdlog::get("app")->error("CONNECTION EXECUTION ERROR : {}", sql_error_message);
      sqlite3_free(sql_error_message);
      return -1;
    }
    return sqlite3_changes(db_instance.get());
  }

  Connection::~Connection() {
    db_instance.reset();
  }
} // namespace database