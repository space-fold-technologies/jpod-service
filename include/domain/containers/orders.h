#ifndef __DAEMON_DOMAIN_CONTAINERS_ORDERS__
#define __DAEMON_DOMAIN_CONTAINERS_ORDERS__

#include <string>
#include <map>

namespace domain::containers
{
    class container_details
    {
        std::string image;
        std::string tag;
        std::string name;
        std::map<std::string, std::string> port_map;
        std::map<std::string, std::string> env_vars;
    };
}

#endif // __DAEMON_DOMAIN_CONTAINERS_ORDERS__