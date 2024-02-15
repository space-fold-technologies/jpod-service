#ifndef __DAEMON_CORE_SQL_STATEMENT__
#define __DAEMON_CORE_SQL_STATEMENT__

#include <string>
#include <memory>
#include <vector>

struct sqlite3_stmt;

namespace spdlog
{
    class logger;
};

namespace core::sql::internal
{
    class connection;
};

namespace core::sql
{
    class connection_proxy;
    class transaction;
    class result_set;
    class statement : public std::enable_shared_from_this<statement>
    {
        friend class result_set;
        friend class internal::connection;

    public:
        explicit statement(internal::connection *connection, const std::string &query);
        explicit statement(connection_proxy &proxy, const std::string &query);
        explicit statement(transaction &txn, const std::string &query);
        virtual ~statement();
        void reset();
        void bind(const int index, const std::string &value) noexcept;
        void bind(const int index, const int value) noexcept;
        void bind(const int index, const int64_t value) noexcept;
        void bind(const int index, const double value) noexcept;
        void bind(const int index, bool value) noexcept;
        void bind(const int index, const std::vector<uint8_t>& content) noexcept;
        void clear() noexcept;
        int execute() noexcept;
        result_set execute_query() noexcept;

    private:
        internal::connection *connection;
        const std::string &query;
        sqlite3_stmt *instance;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_CORE_SQL_STATEMENT__