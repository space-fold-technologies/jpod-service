#ifndef __DAEMON_DOMAIN_NETWORKING_MAPPINGS__
#define __DAEMON_DOMAIN_NETWORKING_MAPPINGS__

#include <string>

namespace domain::networking
{
    struct network_entry
    {
        std::string name;
        std::string driver;
        std::string scope;
        std::string subnet;
    };
}

#endif //__DAEMON_DOMAIN_NETWORKING_MAPPINGS__