#include <core/sql/transaction.h>
#include <core/sql/connection_proxy.h>
#include <spdlog/spdlog.h>

namespace core::sql
{
    transaction::transaction(connection_proxy &proxy) : proxy(proxy), committed(false), logger(spdlog::get("jpod"))
    {
        if (proxy->execute("BEGIN"))
        {
            logger->debug("transaction started");
        }
    }
    void transaction::commit()
    {
        if (!committed)
        {
            if (proxy->execute("COMMIT") > 0)
            {
                committed = true;
                logger->debug("transaction committed");
            }
        }
    }

    transaction::~transaction()
    {
        if (!committed)
        {
            proxy->execute("ROLLBACK");
            logger->debug("transaction rolled back");
        }
    }
}
