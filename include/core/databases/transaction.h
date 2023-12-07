#ifndef __JPOD_CORE_DATABASES_TRANSACTION__
#define __JPOD_CORE_DATABASES_TRANSACTION__

#include <memory>

namespace spdlog
{
    class logger;
}

namespace core::databases
{
    class ConnectionProxy;
    class Connection;
    class Statement;
    class Transaction
    {
        friend class Statement;

    public:
        Transaction(ConnectionProxy &proxy);
        virtual ~Transaction();
        void commit();

    private:
        ConnectionProxy &proxy;
        bool committed;
        std::shared_ptr<spdlog::logger> logger;
    };
};     // namespace core::databases
#endif // __JPOD_CORE_DATABASES_TRANSACTION__