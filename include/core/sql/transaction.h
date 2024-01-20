#ifndef __DAEMON_CORE_SQL_TRANSACTION__
#define __DAEMON_CORE_SQL_TRANSACTION__

#include <memory>

namespace spdlog
{
    class logger;
}

namespace core::sql
{
    class connection_proxy;
    class transaction
    {
        friend class statement;

    public:
        transaction(connection_proxy &proxy);
        virtual ~transaction();
        void commit();

    private:
        connection_proxy &proxy;
        bool committed;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_CORE_SQL_TRANSACTION__