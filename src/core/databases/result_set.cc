#include <core/databases/result_set.h>
#include <core/databases/statement.h>
#include <sqlite3.h>

namespace core::databases
{
    ResultSet::ResultSet(const std::shared_ptr<Statement> statement) : statement(statement)
    {
    }
    int32_t ResultSet::column_index(const std::string &column_name) const
    {
        return sqlite3_bind_parameter_index(this->statement->instance, column_name.c_str());
    }
    int32_t ResultSet::fetch_integer(const int column_index) const
    {
        return sqlite3_column_int(this->statement->instance, column_index);
    }
    int64_t ResultSet::fetch_long(const int column_index) const
    {
        return sqlite3_column_int64(this->statement->instance, column_index);
    }
    double ResultSet::fetch_double(const int column_index) const
    {
        return sqlite3_column_double(this->statement->instance, column_index);
    }
    std::string ResultSet::fetch_string(const int column_index) const
    {
        const char *text = reinterpret_cast<const char *>(sqlite3_column_text(this->statement->instance, column_index));
        return std::string(text);
    }
    bool ResultSet::has_next()
    {
        return sqlite3_step(this->statement->instance) == SQLITE_ROW;
    }
}