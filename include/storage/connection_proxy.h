#ifndef __JPOD_SQLITE3_CONNECTION_PROXY__
#define __JPOD_SQLITE3_CONNECTION_PROXY__

#include <mutex>
#include <storage/connection.h>
#include <storage/data_source.h>

namespace database {

  class ConnectionProxy final {
    friend class DataSource;

  public:
    ConnectionProxy(ConnectionProxy &&other) noexcept : connection(other.connection), dataSource(other.dataSource) {
      other.connection = nullptr;
      other.dataSource = nullptr;
    }
    ConnectionProxy &operator=(ConnectionProxy &&other) noexcept {
      if (this == &other) {
        // prevent self-assignment
        return *this;
      }

      dataSource = other.dataSource;
      connection = other.connection;
      other.dataSource = nullptr;
      other.connection = nullptr;
      return *this;
    }
    ~ConnectionProxy() {
      dataSource->returnToPool(connection);
    }
    Connection *operator->() { return connection; }

    Connection &operator*() { return *connection; }
    Connection *get() { return connection; }
    bool isValid() {
      if (connection != nullptr && dataSource != nullptr) {
        std::unique_lock lock{dataSource->connection_mutex};
        return dataSource->busy.count(connection) > 0;
      }
      return false;
    }

  private:
    ConnectionProxy(Connection *connection, DataSource *dataSource) : connection(connection), dataSource(dataSource) {}
    Connection *connection;
    DataSource *dataSource;
  };
};     // namespace database
#endif // __JPOD_SQLITE3_CONNECTION_PROXY__