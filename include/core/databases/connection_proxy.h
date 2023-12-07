#ifndef __JPOD_CORE_DATABASES_CONNECTION_PROXY__
#define __JPOD_CORE_DATABASES_CONNECTION_PROXY__

#include <core/databases/connection.h>
#include <core/databases/data_source.h>
#include <mutex>

namespace core::databases
{
    class ConnectionProxy
    {
        friend class DataSource;

    public:
        ConnectionProxy(ConnectionProxy &&other) noexcept : connection(other.connection), data_source(other.data_source)
        {
            other.connection = nullptr;
            other.data_source = nullptr;
        }
        ConnectionProxy &operator=(ConnectionProxy &&other) noexcept
        {
            if (this == &other)
            {
                // prevent self-assignment
                return *this;
            }

            data_source = other.data_source;
            connection = other.connection;
            other.data_source = nullptr;
            other.connection = nullptr;
            return *this;
        }
        ~ConnectionProxy()
        {
            data_source->back_to_pool(connection);
        }
        Connection *operator->() { return connection; }

        Connection &operator*() { return *connection; }
        Connection *get() { return connection; }
        bool is_valid()
        {
            if (connection != nullptr && data_source != nullptr)
            {
                std::unique_lock lock{data_source->connection_mutex};
                return data_source->busy.count(connection) > 0;
            }
            return false;
        }

    private:
        ConnectionProxy(Connection *connection, DataSource *data_source) : connection(connection), data_source(data_source) {}
        Connection *connection;
        DataSource *data_source;
    };
}
#endif // __JPOD_CORE_DATABASES_CONNECTION_PROXY__