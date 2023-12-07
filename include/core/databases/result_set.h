#ifndef __JPOD_CORE_DATABASES_RESULT_SET__
#define __JPOD_CORE_DATABASES_RESULT_SET__
#include <memory>
#include <string>

namespace core::databases
{
    class Statement;
    class ResultSet
    {
    public:
        explicit ResultSet(const std::shared_ptr<Statement> statement);
        virtual ~ResultSet() = default;
        template <typename T>
        inline T fetch(const std::string &column_name) const
        {
            if constexpr (std::is_same_v<T, std::string>)
            {
                return fetch_string(column_index(column_name));
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                return fetch_double(column_index(column_name));
            }
            else if constexpr (std::is_same_v<T, int32_t>)
            {
                return fetch_integer(column_index(column_name));
            }
            else if constexpr (std::is_same_v<T, int64_t>)
            {
                return fetch_long(column_index(column_name));
            }
        }

        bool has_next();

    private:
        const std::shared_ptr<Statement> statement;
        int column_index(const std::string &column_name) const;
        int32_t fetch_integer(const int column_index) const;
        int64_t fetch_long(const int column_index) const;
        double fetch_double(const int column_index) const;
        std::string fetch_string(const int column_index) const;
    };

}

#endif // __JPOD_CORE_DATABASES_RESULT_SET__