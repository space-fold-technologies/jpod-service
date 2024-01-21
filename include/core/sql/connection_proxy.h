#ifndef __DAEMON_CORE_SQL_CONNECTION_PROXY__
#define __DAEMON_CORE_SQL_CONNECTION_PROXY__

#include <core/sql/connection.h>
#include <core/sql/data_source.h>

namespace core::sql
{
    class connection_proxy
    {
        friend class core::sql::pool::data_source;

    public:
        connection_proxy(connection_proxy &&other) noexcept : connection(other.connection), data_source(other.data_source)
        {
            other.connection = nullptr;
            other.data_source = nullptr;
        }
        connection_proxy &operator=(connection_proxy &&other) noexcept
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
        ~connection_proxy()
        {
            data_source->back_to_pool(connection);
        }
        internal::connection *operator->() { return connection; }

        internal::connection &operator*() { return *connection; }
        internal::connection *get() { return connection; }
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
        connection_proxy(internal::connection *connection, pool::data_source *data_source) : connection(connection), data_source(data_source) {}

    private:
        internal::connection *connection;
        pool::data_source *data_source;
    };
}

#endif // __DAEMON_CORE_SQL_CONNECTION_PROXY__