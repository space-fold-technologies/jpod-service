#ifndef __DAEMON_DOMAIN_NETWORKING_IP_ADDRESS_PROVIDER__
#define __DAEMON_DOMAIN_NETWORKING_IP_ADDRESS_PROVIDER__

#include <domain/networking/details.h>
#include <optional>
#include <system_error>

namespace domain::networking
{
    class ip_address_provider
    {
    public:
        virtual ~ip_address_provider() = default;
        virtual void initialize(std::error_code &error) = 0;
        virtual std::optional<ip_address> fetch_next_available(ip_address_type type, std::error_code &error) = 0;
        virtual bool remove(const std::string &address_identifier) = 0;
    };
}

#endif //__DAEMON_DOMAIN_NETWORKING_IP_ADDRESS_PROVIDER__