#include <core/sql/result_set.h>
#include <core/sql/statement.h>
#include <cstring>
#include <sqlite3.h>

namespace core::sql
{
    result_set::result_set(const std::shared_ptr<statement> statement_ptr) : statement_ptr(statement_ptr)
    {
    }
    int32_t result_set::column_index(const std::string &column_name) const
    {
        return sqlite3_bind_parameter_index(this->statement_ptr->instance, column_name.c_str());
    }
    int32_t result_set::fetch_integer(const int column_index) const
    {
        return sqlite3_column_int(this->statement_ptr->instance, column_index);
    }
    int64_t result_set::fetch_long(const int column_index) const
    {
        return sqlite3_column_int64(this->statement_ptr->instance, column_index);
    }
    double result_set::fetch_double(const int column_index) const
    {
        return sqlite3_column_double(this->statement_ptr->instance, column_index);
    }
    std::string result_set::fetch_string(const int column_index) const
    {
        const char *text = reinterpret_cast<const char *>(sqlite3_column_text(this->statement_ptr->instance, column_index));
        return std::string(text);
    }
    std::vector<uint8_t> result_set::fetch_blob(const int column_index) const
    {
        auto size = sqlite3_column_bytes(this->statement_ptr->instance, column_index);
        std::vector<uint8_t> content;
        content.reserve(size);
        const void *raw_ptr = sqlite3_column_blob(this->statement_ptr->instance, column_index);
        std::memcpy(content.data(), raw_ptr, size);
        //need to know if sqlite will free the pointer
        return content;
    }
    bool result_set::has_next()
    {
        return sqlite3_step(this->statement_ptr->instance) == SQLITE_ROW;
    }
}
