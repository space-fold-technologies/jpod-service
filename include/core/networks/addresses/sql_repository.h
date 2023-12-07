#ifndef __JPOD_SERVICE_CORE_NETWORKS_ADDRESSES_SQL_REPOSITORY__
#define __JPOD_SERVICE_CORE_NETWORKS_ADDRESSES_SQL_REPOSITORY__

#include <core/networks/addresses/repository.h>
namespace core::networks::addresses
{
    class SQLIPAddressRepository : public IPAddressRepository
    {
    public:
        SQLIPAddressRepository();
        virtual ~SQLIPAddressRepository() = default;
        std::vector<std::string> fetch_used_addresses() override;
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_ADDRESSES_SQL_REPOSITORY__