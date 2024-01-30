#ifndef __DAEMON_DOMAIN_CONTAINERS_DETAILS__
#define __DAEMON_DOMAIN_CONTAINERS_DETAILS__

#include <domain/images/mappings.h>
namespace domain::containers
{
    struct container_details
    {
        std::string identifier;
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> env_vars;
        std::vector<domain::images::mount_point> mount_points;
    };
    struct container_properties
    {
        std::string identifier;
        std::string name;
        std::string image_identifier;
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> port_map;
        std::map<std::string, std::string> env_vars;
        std::string network_properties;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_DETAILS__