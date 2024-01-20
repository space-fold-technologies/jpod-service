#include <core/sql/statement.h>
#include <core/sql/result_set.h>
#include <core/sql/transaction.h>
#include <core/sql/connection.h>
#include <core/sql/connection_proxy.h>
#include <spdlog/spdlog.h>
#include <sqlite3.h>

namespace core::sql
{
  statement::statement(internal::connection *connection, const std::string &query) : connection(connection),
                                                                                     query(query),
                                                                                     instance(nullptr),
                                                                                     logger(spdlog::get("jpod"))
  {
    if (sqlite3_prepare_v2(this->connection->handle(), this->query.c_str(), -1, &this->instance, nullptr) != SQLITE_OK)
    {
      logger->error("FAILED TO DO BIND {}", sqlite3_errmsg(connection->handle()));
    }
  }

  statement::statement(connection_proxy &proxy, const std::string &query) : statement((proxy.get()), query) {}

  statement::statement(transaction &txn, const std::string &query) : statement(txn.proxy, query) {}

  void statement::reset()
  {
    if (instance != nullptr)
    {
      sqlite3_reset(this->instance);
    }
  }

  void statement::bind(const int index, const std::string &value) noexcept
  {
    if (sqlite3_bind_text(this->instance, index, value.c_str(), value.size(), SQLITE_STATIC) != SQLITE_OK)
    {
      logger->error("STRING BIND FAILED");
    }
  }

  void statement::bind(const int index, const int value) noexcept
  {
    sqlite3_bind_int(this->instance, index, value);
  }
  void statement::bind(const int index, const int64_t value) noexcept
  {
    sqlite3_bind_int64(this->instance, index, value);
  }
  void statement::bind(const int index, const double value) noexcept
  {
    sqlite3_bind_double(this->instance, index, value);
  }
  void statement::clear() noexcept
  {
    if (instance != nullptr)
    {
      sqlite3_clear_bindings(this->instance);
    }
  }
  int statement::execute() noexcept
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

  void statement::bind(const int index, const std::vector<uint8_t> &content) noexcept
  {
    if (sqlite3_bind_blob(this->instance, index, content.data(), content.size(), SQLITE_STATIC) != SQLITE_OK)
    {
      logger->error("bind blob failed");
    }
  }

  result_set statement::execute_query() noexcept
  {
    return result_set(shared_from_this());
  }

  statement::~statement()
  {
    if (instance != nullptr)
    {
      sqlite3_finalize(instance);
    }
  }
}
