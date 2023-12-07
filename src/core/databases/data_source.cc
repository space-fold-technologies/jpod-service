#include <core/databases/data_source.h>
#include <core/databases/connection_proxy.h>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <sqlite3.h>

namespace core::databases
{
  DataSource::DataSource(std::string path, std::size_t pool_size) : path(path),
                                                                    pool_size(pool_size),
                                                                    logger(spdlog::get("jpod"))
  {
    logger->info("SQLITE {}", sqlite3_libversion());
    logger->info("IS THREAD SAFE {}", sqlite3_threadsafe() ? "ON" : "OFF");
  }

  ConnectionProxy DataSource::connection()
  {
    if (connection_mutex.try_lock())
    {
      for (auto &[key, value] : available)
      {
        ConnectionProxy proxy{key, this};
        auto node = available.extract(key);
        busy.insert(std::move(node));
        connection_mutex.unlock();
        return proxy;
      }
    }
    return ConnectionProxy{nullptr, nullptr};
  }

  void DataSource::initialize()
  {
    while (pool_size > available.size())
    {
      auto connection = std::unique_ptr<Connection>(new Connection(path));
      connection->open();
      auto key = connection.get();
      available.try_emplace(key, std::move(connection));
    }
  }

  void DataSource::back_to_pool(Connection *instance)
  {
    if (connection_mutex.try_lock())
    {
      if (auto result = busy.find(instance); result != busy.end())
      {
        auto node = busy.extract(result);
        available.insert(std::move(node));
      }
      connection_mutex.unlock();
    }
  }

  DataSource::~DataSource() {}
} // namespace core::databases