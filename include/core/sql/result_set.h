#ifndef __DAEMON_CORE_SQL_RESULT_SET__
#define __DAEMON_CORE_SQL_RESULT_SET__

#include <type_traits>
#include <memory>
#include <string>
#include <vector>
#include <chrono>

using namespace std::chrono;
namespace core::sql
{
    class statement;
    class result_set
    {
    public:
        explicit result_set(const std::shared_ptr<statement> statement);
        virtual ~result_set() = default;
        template <typename T, std::enable_if_t<std::is_same<T, std::string>::value> * = nullptr>
        inline T fetch(const std::string &column_name) const
        {
            return fetch_string(column_index(column_name));
        }
        template <typename T, std::enable_if_t<std::is_same<T, double>::value> * = nullptr>
        inline T fetch(const std::string &column_name) const
        {
            return fetch_double(column_index(column_name));
        }
        template <typename T, std::enable_if_t<std::is_same<T, int32_t>::value> * = nullptr>
        inline T fetch(const std::string &column_name) const
        {
            return fetch_integer(column_index(column_name));
        }
        template <typename T, std::enable_if_t<std::is_same<T, int64_t>::value> * = nullptr>
        inline T fetch(const std::string &column_name) const
        {
            return fetch_long(column_index(column_name));
        }
        template <typename T, std::enable_if_t<std::is_same<T, bool>::value> * = nullptr>
        inline T fetch(const std::string &column_name) const
        {
            return fetch_bool(column_index(column_name));
        }
        template <typename T, std::enable_if_t<std::is_same<T, std::vector<uint8_t>>::value> * = nullptr>
        inline T fetch(const std::string &column_name) const
        {
            return fetch_blob(column_index(column_name));
        }
        template <typename T, std::enable_if_t<std::is_same<T, time_point<system_clock, nanoseconds>>::value> * = nullptr>
        inline T fetch(const std::string &column_name) const
        {
            return fetch_timestamp(column_index(column_name));
        }

        bool has_next();

    private:
        const std::shared_ptr<statement> statement_ptr;
        int column_index(const std::string &column_name) const;
        int32_t fetch_integer(const int column_index) const;
        int64_t fetch_long(const int column_index) const;
        double fetch_double(const int column_index) const;
        bool fetch_bool(const int column_index) const;
        std::string fetch_string(const int column_index) const;
        std::vector<uint8_t> fetch_blob(const int column_index) const;
        time_point<system_clock, nanoseconds> fetch_timestamp(const int column_index) const;
    };
}

#endif // __DAEMON_CORE_SQL_RESULT_SET__