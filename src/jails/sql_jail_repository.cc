#include <jails/sql_jail_repository.h>
#include <spdlog/spdlog.h>
#include <storage/connection_proxy.h>
#include <storage/data_source.h>
#include <storage/resultset.h>
#include <storage/statement.h>
#include <storage/transaction.h>

namespace prison {
  using namespace database;
  SqlPrisonRepository::SqlPrisonRepository(database::DataSource &dataSource) : dataSource(dataSource) {
  }

  void SqlPrisonRepository::save(const Details &details) {

    std::string sql("INSERT INTO prison(jid, name, reference, properties, status) "
                    "VALUES(?, ?, ?, ?, ?)");
    if (auto connectionProxy = dataSource.fetchConnection(); connectionProxy.isValid()) {
      database::Transaction transaction(connectionProxy);
      database::Statement statement(transaction, sql);
      statement.bind(1, details.jid);
      statement.bind(2, details.name);
      statement.bind(3, details.reference);
      statement.bind(4, details.properties);
      statement.bind(5, details.status);
      if (statement.execute() > 0) {
        transaction.commit();
        spdlog::get("app")->info("PERSISTED");
      }
    }
  }

  Details SqlPrisonRepository::fetchByReference(const std::string &reference) {
    auto logger = spdlog::get("app");
    Details details;
    if (auto connectionProxy = dataSource.fetchConnection(); connectionProxy.isValid()) {
      std::string sql("SELECT jid, name, reference, properties, status FROM prison WHERE reference=?");
      database::Statement statement(connectionProxy, sql);
      statement.bind(0, reference);
      if (auto resultset = statement.executeQuery(); resultset.hasNext()) {
        details.jid = resultset.fetch<int>("jid");
        details.name = resultset.fetch<std::string>("name");
        details.reference = resultset.fetch<std::string>("reference");
        details.properties = resultset.fetch<std::string>("properties");
        details.status = resultset.fetch<std::string>("status");
      }
      statement.clear();
      statement.reset();
    }
    return details;
  }

  Details SqlPrisonRepository::fetchByNameOrReference(const std::string &value) {
    Details details;
    if (auto connectionProxy = dataSource.fetchConnection(); connectionProxy.isValid()) {
      std::string sql("SELECT jid, name, reference, properties, status FROM prison WHERE reference=? OR name=?");
      database::Statement statement(connectionProxy, sql);
      statement.bind(0, value);
      statement.bind(1, value);
      if (auto resultset = statement.executeQuery(); resultset.hasNext()) {
        details.jid = resultset.fetch<int>("jid");
        details.name = resultset.fetch<std::string>("name");
        details.reference = resultset.fetch<std::string>("reference");
        details.properties = resultset.fetch<std::string>("properties");
        details.status = resultset.fetch<std::string>("status");
      }
      statement.clear();
      statement.reset();
    }
    return details;
  }

  std::vector<Details> SqlPrisonRepository::fetchAll() {

    std::vector<Details> details;
    if (auto connectionProxy = dataSource.fetchConnection(); connectionProxy.isValid()) {
      std::string sql("SELECT jid, name, reference, properties, status FROM prison");
      database::Statement statement(connectionProxy, sql);
      auto resultset = statement.executeQuery();
      while (resultset.hasNext()) {
        Details detail;
        detail.jid = resultset.fetch<int>("jid");
        detail.name = resultset.fetch<std::string>("name");
        detail.reference = resultset.fetch<std::string>("reference");
        detail.properties = resultset.fetch<std::string>("properties");
        detail.status = resultset.fetch<std::string>("status");
        details.push_back(detail);
      }
      statement.clear();
      statement.reset();
    }
    return details;
  }

  void SqlPrisonRepository::remove(const std::string &reference) {
    if (auto connectionProxy = dataSource.fetchConnection(); connectionProxy.isValid()) {
      std::string sql("DELETE FROM prison WHERE reference=?");
      database::Transaction transaction(connectionProxy);
      database::Statement statement(transaction, sql);
      statement.bind(0, reference);
      if (statement.execute() < 0) {
        spdlog::get("app")->error("Did not delete prison successfully");
      } else {
        transaction.commit();
      }
      statement.clear();
      statement.reset();
    }
  }

  SqlPrisonRepository::~SqlPrisonRepository() {
  }
}; // namespace prison
