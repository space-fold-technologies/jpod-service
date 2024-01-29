#ifndef __DAEMON_DOMAIN_CONTAINERS_ORDERS__
#define __DAEMON_DOMAIN_CONTAINERS_ORDERS__

#include <msgpack/msgpack.hpp>

namespace domain::containers
{
    class container_creation_order
    {
        std::string tagged_image;
        std::string name;
        std::map<std::string, std::string> port_map;
        std::map<std::string, std::string> env_vars;

        template <class T>
        void pack(T &pack) 
        {
             pack(image, tag, name, port_map, env_vars);
        }
    };

     inline container_creation_order unpack_container_creation_order(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<container_creation_order>(content);
    }
}

#endif // __DAEMON_DOMAIN_CONTAINERS_ORDERS__