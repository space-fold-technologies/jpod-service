#ifndef __JPOD_CORE_DATABASES_DATABASE_SOURCE__
#define __JPOD_CORE_DATABASES_DATABASE_SOURCE__

#include <map>
#include <memory>
#include <mutex>

namespace spdlog
{
    class logger;
};

namespace core::databases
{
    class Connection;
    class ConnectionProxy;
    class DataSource
    {
        friend class ConnectionProxy;

    public:
        explicit DataSource(std::string path, std::size_t pool_size);
        virtual ~DataSource();
        ConnectionProxy connection();
        void initialize();

    private:
        void back_to_pool(Connection *connection);

    private:
        std::string path;
        std::size_t pool_size;
        std::mutex connection_mutex;
        std::map<Connection *, std::unique_ptr<Connection>> available;
        std::map<Connection *, std::unique_ptr<Connection>> busy;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __JPOD_CORE_DATABASES_DATABASE_SOURCE__