#include <core/networks/addresses/sql_repository.h>

namespace core::networks::addresses
{
    SQLIPAddressRepository::SQLIPAddressRepository()
    {
    }
    std::vector<std::string> SQLIPAddressRepository::fetch_used_addresses()
    {
        return {};
    }
}