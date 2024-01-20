#ifndef __DAEMON_CORE_SQL_DATA_SOURCE__
#define __DAEMON_CORE_SQL_DATA_SOURCE__

#include <map>
#include <memory>
#include <mutex>

namespace spdlog
{
    class logger;
};

namespace core::sql
{
    class connection_proxy;
    namespace internal
    {
        class connection;
    };
};

namespace in = core::sql::internal;

namespace core::sql::pool
{
    class data_source
    {
        friend class core::sql::connection_proxy;

    public:
        explicit data_source(std::string path, std::size_t pool_size);
        virtual ~data_source();
        core::sql::connection_proxy connection();
        void initialize();

    private:
        void back_to_pool(in::connection *connection);

    private:
        std::string path;
        std::size_t pool_size;
        std::mutex connection_mutex;
        std::map<in::connection *, std::unique_ptr<in::connection>> available;
        std::map<in::connection *, std::unique_ptr<in::connection>> busy;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_CORE_SQL_DATA_SOURCE__