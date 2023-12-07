#ifndef __JPOD_SERVICE_CORE_NETWORKS_ADDRESSES_REPOSITORY__
#define __JPOD_SERVICE_CORE_NETWORKS_ADDRESSES_REPOSITORY__

#include <string>
#include <vector>

namespace core::networks::addresses
{
    class IPAddressRepository
    {
    public:
        virtual std::vector<std::string> fetch_used_addresses() = 0;
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_ADDRESSES_REPOSITORY__
