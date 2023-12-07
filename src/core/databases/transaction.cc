#include <core/databases/transaction.h>
#include <core/databases/connection_proxy.h>
#include <spdlog/spdlog.h>

namespace core::databases
{
    Transaction::Transaction(ConnectionProxy &proxy) : proxy(proxy), committed(false)
    {
        if (proxy->execute("BEGIN"))
        {
            logger->debug("Transaction started");
        }
    }
    void Transaction::commit()
    {
        if (!committed)
        {
            if (proxy->execute("COMMIT") > 0)
            {
                committed = true;
                logger->debug("Transaction committed");
            }
        }
    }

    Transaction::~Transaction()
    {
        if (!committed)
        {
            proxy->execute("ROLLBACK");
            logger->debug("Transaction rolled back");
        }
    }
}; // namespace core::database