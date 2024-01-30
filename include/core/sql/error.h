#ifndef __DAEMON_CORE_SQL_ERRORS__
#define __DAEMON_CORE_SQL_ERRORS__

#include <sqlite3.h>
#include <system_error>
#include <fmt/format.h>

namespace core::sql::errors
{
    struct sql_failure_category : public std::error_category
    {
        sql_failure_category() {}
        virtual ~sql_failure_category() = default;
        sql_failure_category(const sql_failure_category &) = delete;
        const char *name() const noexcept override
        {
            return "sql failures";
        }

        std::string message(int ec) const override
        {
            return fmt::format("{}", sqlite3_errstr(ec));
        }
    };

    inline const sql_failure_category &__category()
    {
        static sql_failure_category fc;
        return fc;
    }

    inline const std::error_code make_error_code(int ec) noexcept
    {

        return std::error_code{ec, __category()};
    };
}

#endif // __DAEMON_CORE_SQL_ERRORS__