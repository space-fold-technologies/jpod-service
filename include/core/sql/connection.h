#ifndef __DAEMON_CORE_SQL_CONNECTION__
#define __DAEMON_CORE_SQL_CONNECTION__

#include <functional>
#include <memory>
#include <string>

struct sqlite3;

namespace spdlog
{
    class logger;
};

namespace core::sql
{
    class statement;
};

namespace core::sql::internal
{
    template <typename T>
    using database_ptr = std::unique_ptr<T, std::function<void(T *)>>;
    class connection
    {
        friend class core::sql::statement;

    public:
        explicit connection(const std::string &path);
        virtual ~connection();
        bool has_table(const std::string &name);
        bool is_ok();
        int execute(const std::string &query);
        void open();
        core::sql::statement statement(const std::string& sql);

    private:
        inline sqlite3 *handle()
        {
            return instance.get();
        }

    private:
        const std::string &path;
        database_ptr<sqlite3> instance;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_CORE_SQL_CONNECTION__