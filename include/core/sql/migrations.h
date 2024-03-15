#ifndef __DAEMON_CORE_SQL_MIGRATIONS__
#define __DAEMON_CORE_SQL_MIGRATIONS__

#include <system_error>
#include <string>
#include <memory>
#include <map>

namespace spdlog
{
    class logger;
};

namespace core::sql::pool
{
    class data_source;
};

namespace core::sql
{
    class connection_proxy;
    class migration_handler
    {
    public:
        explicit migration_handler(pool::data_source &data_source, std::string path);
        virtual ~migration_handler();
        void migrate();

    private:
        std::error_code apply(const std::string &content, const std::string &migration);
        bool create_table();
        bool has_table();
        bool has_been_done(connection_proxy &proxy, const std::string &migration);
        std::error_code register_migration(connection_proxy &proxy, const std::string &migration);
        std::error_code register_state(connection_proxy &proxy, const std::string &migration, bool state);

    private:
        pool::data_source &data_source;
        std::string path;
        std::map<int, std::string> migrations;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_CORE_SQL_MIGRATIONS__