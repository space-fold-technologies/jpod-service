#ifndef __JPOD_CORE_DATABASES_STATEMENT__
#define __JPOD_CORE_DATABASES_STATEMENT__

#include <string>
#include <memory>

struct sqlite3_stmt;

namespace spdlog
{
    class logger;
};

namespace core::databases
{
    class ConnectionProxy;
    class Connection;
    class Transaction;
    class ResultSet;
    class Statement : public std::enable_shared_from_this<Statement>
    {
        friend class ResultSet;
        friend class Connection;

    public:
        Statement(Connection *connection, const std::string &query);
        Statement(ConnectionProxy &proxy, const std::string &query);
        Statement(Transaction &transaction, const std::string &query);
        ~Statement();
        void reset();
        void bind(const int index, const std::string &value) noexcept;
        void bind(const int index, const int value) noexcept;
        void bind(const int index, const int64_t value) noexcept;
        void bind(const int index, const double value) noexcept;
        void clear() noexcept;
        int execute() noexcept;
        ResultSet execute_query() noexcept;

    private:
        Connection *connection;
        const std::string &query;
        sqlite3_stmt *instance;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __JPOD_CORE_DATABASES_STATEMENT__