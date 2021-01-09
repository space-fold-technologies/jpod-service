#include <filesystem>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <storage/connection_proxy.h>
#include <storage/data_source.h>

namespace database {
  DataSource::DataSource(const std::string &path, int poolsize)
      : path(path), poolsize(poolsize) {
    spdlog::get("app")->info("SQLITE {}", sqlite3_libversion());
    spdlog::get("app")->info("IS THREAD SAFE {}", sqlite3_threadsafe() ? "ON" : "OFF");
  }

  ConnectionProxy database::DataSource::fetchConnection() {
    if (connection_mutex.try_lock()) {
      for (auto &[key, value] : available) {
        ConnectionProxy proxy{key, this};
        auto node = available.extract(key);
        busy.insert(std::move(node));
        connection_mutex.unlock();
        return proxy;
      }
    }
    return ConnectionProxy{nullptr, nullptr};
  }

  void DataSource::initialize() {
    while (poolsize > static_cast<int>(available.size())) {
      auto connection = std::unique_ptr<Connection>(new Connection(path));
      connection->open();
      auto key = connection.get();
      available.try_emplace(key, std::move(connection));
    }
  }

  void DataSource::returnToPool(Connection *instance) {
    if (connection_mutex.try_lock()) {
      if (auto result = busy.find(instance); result != busy.end()) {
        auto node = busy.extract(result);
        available.insert(std::move(node));
      }
      connection_mutex.unlock();
    }
  }

  database::DataSource::~DataSource() {}
} // namespace database