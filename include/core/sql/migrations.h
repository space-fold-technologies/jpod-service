#ifndef __DAEMON_CORE_SQL_MIGRATIONS__
#define __DAEMON_CORE_SQL_MIGRATIONS__

#include <string>
#include <memory>
#include <system_error>

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
    class migration_handler
    {
        explicit migration_handler(pool::data_source &data_source, const std::string &folder);
        virtual ~migration_handler();
        void migrate();

    private:
        void apply(const std::string &content, const std::string &migration);

    private:
        pool::data_source &data_source;
        const std::string &folder;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_CORE_SQL_MIGRATIONS__