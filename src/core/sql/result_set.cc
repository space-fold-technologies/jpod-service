#include <core/sql/result_set.h>
#include <core/sql/statement.h>
#include <cstring>
#include <sqlite3.h>

namespace core::sql
{
    result_set::result_set(statement &stmt) : stmt(stmt)
    {
    }
    int32_t result_set::column_index(const std::string &column_name) const
    {
        return sqlite3_bind_parameter_index(stmt.instance, column_name.c_str());
    }
    int32_t result_set::fetch_integer(const int column_index) const
    {
        return sqlite3_column_int(stmt.instance, column_index);
    }
    int64_t result_set::fetch_long(const int column_index) const
    {
        return sqlite3_column_int64(stmt.instance, column_index);
    }
    double result_set::fetch_double(const int column_index) const
    {
        return sqlite3_column_double(stmt.instance, column_index);
    }
    bool result_set::fetch_bool(const int column_index) const
    {
        return sqlite3_column_int(stmt.instance, column_index) > 0;
    }
    std::string result_set::fetch_string(const int column_index) const
    {
        const char *text = reinterpret_cast<const char *>(sqlite3_column_text(stmt.instance, column_index));
        return std::string(text);
    }
    std::vector<uint8_t> result_set::fetch_blob(const int column_index) const
    {
        auto size = sqlite3_column_bytes(stmt.instance, column_index);
        std::vector<uint8_t> content;
        content.reserve(size);
        const void *raw_ptr = sqlite3_column_blob(stmt.instance, column_index);
        std::memcpy(content.data(), raw_ptr, size);
        // need to know if sqlite will free the pointer
        return content;
    }
    time_point<system_clock, nanoseconds> result_set::fetch_timestamp(const int column_index) const
    {
        auto timestamp = seconds(sqlite3_column_int64(stmt.instance, column_index));
        auto value = system_clock::from_time_t(timestamp.count());
        return time_point_cast<nanoseconds>(value);
    }
    bool result_set::has_next()
    {
        return sqlite3_step(stmt.instance) == SQLITE_ROW;
    }
}
