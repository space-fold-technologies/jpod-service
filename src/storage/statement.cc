#include <any>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <storage/connection_proxy.h>
#include <storage/resultset.h>
#include <storage/statement.h>
#include <storage/transaction.h>

namespace database {

  Statement::Statement(Connection *connection, const std::string &query) : connection(connection),
                                                                           query(query),
                                                                           statement_ptr(nullptr) {
    if (sqlite3_prepare_v2(this->connection->handle(), this->query.c_str(), -1, &this->statement_ptr, nullptr) != SQLITE_OK) {
      spdlog::get("app")->error("FAILED TO DO BIND {}", sqlite3_errmsg(connection->handle()));
    }
  }

  Statement::Statement(ConnectionProxy &connectionProxy, const std::string &query) : Statement((connectionProxy.get()), query) {}

  Statement::Statement(Transaction &transaction, const std::string &query) : Statement(transaction.getConnectionProxy(), query) {}

  void Statement::reset() {
    if (statement_ptr != nullptr) {
      sqlite3_reset(this->statement_ptr);
    }
  }

  void Statement::bind(const int index, const std::string &value) {
    if (sqlite3_bind_text(this->statement_ptr, index, value.c_str(), value.size(), SQLITE_STATIC) != SQLITE_OK) {
      spdlog::get("app")->error("STRING BIND FAILED");
    }
  }

  void Statement::bind(const int index, const int value) {
    sqlite3_bind_int(this->statement_ptr, index, value);
  }
  void Statement::bind(const int index, const int64_t value) {
    sqlite3_bind_int64(this->statement_ptr, index, value);
  }
  void Statement::bind(const int index, const double value) {
    sqlite3_bind_double(this->statement_ptr, index, value);
  }
  void Statement::clear() {
    if (statement_ptr != nullptr) {
      sqlite3_clear_bindings(this->statement_ptr);
    }
  }
  int Statement::execute() noexcept {
    sqlite3_mutex_enter(sqlite3_db_mutex(this->connection->handle()));
    if (sqlite3_step(this->statement_ptr) != SQLITE_DONE) {
      const char *error = sqlite3_errmsg(this->connection->handle());
      int code = sqlite3_errcode(this->connection->handle());
      spdlog::get("app")->error("EXECUTION ERROR : {} CODE : {}", error, code);
    }
    sqlite3_mutex_leave(sqlite3_db_mutex(this->connection->handle()));
    return sqlite3_changes(connection->handle());
  }

  // template <typename T>
  // T Statement::fetch(const std::string &columnName) const {
  //   static_assert(std::is_same<T, std::string>::value ||
  //                     std::is_same<T, double>::value ||
  //                     std::is_same<T, int>::value ||
  //                     std::is_same<T, int64_t>::value,
  //                 "This type cannot be bound");
  //   return std::any_cast<T>(val);
  // }

  ResultSet Statement::executeQuery() noexcept {
    return ResultSet(this);
  }

  Statement::~Statement() {
    if (statement_ptr != nullptr) {
      sqlite3_finalize(statement_ptr);
    }
  }

} // namespace database
