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
        std::map<std::string, std::string> port_map;
        std::vector<domain::images::mount_point> mount_points;
        std::string entry_point;
        std::string network_properties;
    };

    struct container_internals
    {
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> port_map;
        std::map<std::string, std::string> env_vars;
        std::string network_properties;
        template <class T>
        void pack(T &pack)
        {
            pack(parameters, port_map, env_vars, network_properties);
        }
    };

    inline container_internals unpack_container_internals(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<container_internals>(content);
    }

    inline std::vector<uint8_t> pack_container_internals(container_internals &order)
    {
        return msgpack::pack(order);
    }

    inline void fill_container_details(container_details &details, const container_internals &internals)
    {
        details.parameters.insert(internals.parameters.begin(), internals.parameters.end());
        details.port_map.insert(internals.port_map.begin(), internals.port_map.end());
        details.env_vars.insert(internals.env_vars.begin(), internals.env_vars.end());
        details.network_properties = internals.network_properties;;
    }

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

     inline void fill_container_properties(container_properties &properties, const container_internals &internals)
    {
        properties.parameters.insert(internals.parameters.begin(), internals.parameters.end());
        properties.port_map.insert(internals.port_map.begin(), internals.port_map.end());
        properties.env_vars.insert(internals.env_vars.begin(), internals.env_vars.end());
        properties.network_properties = internals.network_properties;
    }

}
#endif // __DAEMON_DOMAIN_CONTAINERS_DETAILS__