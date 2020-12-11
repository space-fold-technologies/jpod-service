#ifndef __JPOD_SQLITE3_STATEMENT__
#define __JPOD_SQLITE3_STATEMENT__

#include <string>
struct sqlite3_stmt;
namespace database {
  class ConnectionProxy;
  class Connection;
  class Transaction;
  class ResultSet;
  class Statement {
    friend class ResultSet;

  public:
    Statement(Connection *connection, const std::string &query);
    Statement(ConnectionProxy &connectionProxy, const std::string &query);
    Statement(Transaction &transaction, const std::string &query);
    ~Statement();
    void reset();
    void bind(const int index, const std::string &value);
    void bind(const int index, const int value);
    void bind(const int index, const int64_t value);
    void bind(const int index, const double value);
    void clear();
    int execute() noexcept;
    ResultSet executeQuery() noexcept;

  private:
    Connection *connection;
    const std::string &query;
    sqlite3_stmt *statement_ptr;
  };
};     // namespace database
#endif // __JPOD_SQLITE3_STATEMENT__
