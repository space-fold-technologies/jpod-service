#include <spdlog/spdlog.h>
#include <storage/connection_proxy.h>
#include <storage/transaction.h>

namespace database {
  Transaction::Transaction(ConnectionProxy &connectionProxy) : connectionProxy(connectionProxy), committed(false) {
    if (connectionProxy->execute("BEGIN")) {
      spdlog::get("app")->debug("Transaction started");
    }
  }
  void Transaction::commit() {
    if (!committed) {
      if (connectionProxy->execute("COMMIT") > 0) {
        committed = true;
        spdlog::get("app")->debug("Transaction committed");
      }
    }
  }

  ConnectionProxy &Transaction::getConnectionProxy() {
    return connectionProxy;
  }

  Transaction::~Transaction() {
    if (!committed) {
      connectionProxy->execute("ROLLBACK");
      spdlog::get("app")->debug("Transaction rolled back");
    }
  }
}; // namespace database