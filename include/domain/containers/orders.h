#ifndef __DAEMON_DOMAIN_CONTAINERS_ORDERS__
#define __DAEMON_DOMAIN_CONTAINERS_ORDERS__

#include <domain/images/mappings.h>
#include <string>
#include <map>

namespace domain::containers
{
    class build_order
    {
        std::string tagged_image;
        std::string name;
        std::map<std::string, std::string> port_map;
        std::map<std::string, std::string> env_vars;
    };

    class container_details
    {
        std::string identifier;
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> env_vars;
        std::vector<domain::images::mount_point> mount_points;
    };
}

#endif // __DAEMON_DOMAIN_CONTAINERS_ORDERS__