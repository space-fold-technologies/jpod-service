#ifndef __JPOD_SQLITE3_CONNECTION_PROVIDER__
#define __JPOD_SQLITE3_CONNECTION_PROVIDER__
#include <map>
#include <memory>
#include <mutex>
#include <storage/connection.h>
#include <string>

namespace database {
  class Connection;
  class ConnectionProxy;
  class DataSource {
    friend class ConnectionProxy;

  public:
    DataSource(const std::string &path, int poolsize = 4);
    ~DataSource();
    ConnectionProxy fetchConnection();
    void initialize();

  private:
    void returnToPool(Connection *instance);
    const std::string &path;
    int poolsize;
    std::mutex connection_mutex;
    std::map<Connection *, std::unique_ptr<Connection>> available;
    std::map<Connection *, std::unique_ptr<Connection>> busy;
  };
};     // namespace database
#endif // __JPOD_SQLITE3_CONNECTION_PROVIDER__