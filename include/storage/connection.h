#ifndef __JPOD_SQLITE3_DATABASE_CONNECTION__
#define __JPOD_SQLITE3_DATABASE_CONNECTION__

#include <functional>
#include <memory>
#include <string>

struct sqlite3;

namespace database {
  template <typename T>
  using db_ptr = std::unique_ptr<T, std::function<void(T *)>>;
  class Connection {
  public:
    Connection(const std::string &path);
    ~Connection();
    sqlite3 *handle();
    bool isOK();
    bool hasTable(const std::string &name);
    int execute(const std::string &query);
    void open();

  private:
    db_ptr<sqlite3> db_instance;
    const std::string &path;
  };
} // namespace database
#endif // __JPOD_SQLITE3_DATABASE_CONNECTION__