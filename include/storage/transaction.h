#ifndef __JPOD_SQLITE3_TRANSACTION__
#define __JPOD_SQLITE3_TRANSACTION__

namespace database {
  class ConnectionProxy;
  class Connection;
  class Statement;
  class Transaction {
    friend class Statement;

  public:
    Transaction(ConnectionProxy &connectionProxy);
    ~Transaction();
    void commit();

  private:
    ConnectionProxy &getConnectionProxy();
    ConnectionProxy &connectionProxy;
    bool committed;
  };
};     // namespace database
#endif // __JPOD_SQLITE3_TRANSACTION__