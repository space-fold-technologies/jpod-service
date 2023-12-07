#include <core/databases/statement.h>
#include <core/databases/result_set.h>
#include <core/databases/transaction.h>
#include <any>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <core/databases/connection_proxy.h>

namespace core::databases
{

  Statement::Statement(Connection *connection, const std::string &query) : connection(connection),
                                                                           query(query),
                                                                           instance(nullptr),
                                                                           logger(spdlog::get("jpod"))
  {
    if (sqlite3_prepare_v2(this->connection->handle(), this->query.c_str(), -1, &this->instance, nullptr) != SQLITE_OK)
    {
      logger->error("FAILED TO DO BIND {}", sqlite3_errmsg(connection->handle()));
    }
  }

  Statement::Statement(ConnectionProxy &proxy, const std::string &query) : Statement((proxy.get()), query) {}

  Statement::Statement(Transaction &transaction, const std::string &query) : Statement(transaction.proxy, query) {}

  void Statement::reset()
  {
    if (instance != nullptr)
    {
      sqlite3_reset(this->instance);
    }
  }

  void Statement::bind(const int index, const std::string &value) noexcept
  {
    if (sqlite3_bind_text(this->instance, index, value.c_str(), value.size(), SQLITE_STATIC) != SQLITE_OK)
    {
      logger->error("STRING BIND FAILED");
    }
  }

  void Statement::bind(const int index, const int value) noexcept
  {
    sqlite3_bind_int(this->instance, index, value);
  }
  void Statement::bind(const int index, const int64_t value) noexcept
  {
    sqlite3_bind_int64(this->instance, index, value);
  }
  void Statement::bind(const int index, const double value) noexcept
  {
    sqlite3_bind_double(this->instance, index, value);
  }
  void Statement::clear() noexcept
  {
    if (instance != nullptr)
    {
      sqlite3_clear_bindings(this->instance);
    }
  }
  int Statement::execute() noexcept
  {
    sqlite3_mutex_enter(sqlite3_db_mutex(this->connection->handle()));
    if (sqlite3_step(this->instance) != SQLITE_DONE)
    {
      const char *error = sqlite3_errmsg(this->connection->handle());
      int code = sqlite3_errcode(this->connection->handle());
      logger->error("EXECUTION ERROR : {} CODE : {}", error, code);
    }
    sqlite3_mutex_leave(sqlite3_db_mutex(this->connection->handle()));
    return sqlite3_changes(connection->handle());
  }

  ResultSet Statement::execute_query() noexcept
  {
    return ResultSet(shared_from_this());
  }

  Statement::~Statement()
  {
    if (instance != nullptr)
    {
      sqlite3_finalize(instance);
    }
  }

} // namespace core::databases
