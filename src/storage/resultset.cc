#include <sqlite3.h>
#include <storage/resultset.h>
#include <storage/statement.h>

namespace database {
  ResultSet::ResultSet(const Statement *statement) : statement(statement) {}

  int ResultSet::getInt(const int columnIndex) const {
    return sqlite3_column_int(this->statement->statement_ptr, columnIndex);
  }

  int64_t ResultSet::getInt64(const int columnIndex) const {
    return sqlite3_column_int64(this->statement->statement_ptr, columnIndex);
  }

  double ResultSet::getDouble(const int columnIndex) const {
    return sqlite3_column_double(this->statement->statement_ptr, columnIndex);
  }

  std::string ResultSet::getString(const int columnIndex) const {
    const char *text = reinterpret_cast<const char *>(sqlite3_column_text(this->statement->statement_ptr, columnIndex));
    return std::string(text);
  }

  int ResultSet::getColumnIndex(const std::string &name) const {
    return sqlite3_bind_parameter_index(this->statement->statement_ptr, name.c_str());
  }

  bool ResultSet::hasNext() {
    return sqlite3_step(this->statement->statement_ptr) == SQLITE_ROW;
  }

  ResultSet::~ResultSet() {}
} // namespace database