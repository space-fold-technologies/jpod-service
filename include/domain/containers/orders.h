#ifndef __DAEMON_DOMAIN_CONTAINERS_ORDERS__
#define __DAEMON_DOMAIN_CONTAINERS_ORDERS__

#include <msgpack.hpp>

namespace domain::containers
{
    struct container_creation_order
    {
        std::string tagged_image;
        std::string name;
        std::map<std::string, std::string> port_map;
        std::map<std::string, std::string> env_vars;
        std::string network_properties;

        MSGPACK_DEFINE(tagged_image, name, port_map, env_vars, network_properties)
    };

    inline container_creation_order unpack_container_creation_order(const std::vector<uint8_t> &content)
    {
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(content.data()), content.size());
        return result.get().as<container_creation_order>();
    }

    struct container_term_order
    {
        std::string term;

        MSGPACK_DEFINE(term)
    };

    inline container_term_order unpack_container_term_order(const std::vector<uint8_t> &content)
    {
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(content.data()), content.size());
        return result.get().as<container_term_order>();
    }
    namespace shell
    {
        constexpr uint8_t start_session = 0x00;
        constexpr uint8_t terminal_size = 0x01;
        constexpr uint8_t terminal_feed = 0x02;
    }
    struct container_shell_order
    {
        uint8_t type;
        std::vector<uint8_t> data;

        MSGPACK_DEFINE(type, data)
    };

    inline container_shell_order unpack_container_shell_order(const std::vector<uint8_t> &content)
    {
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(content.data()), content.size());
        return result.get().as<container_shell_order>();
    }
    namespace filter
    {
        constexpr uint8_t all = 0x00;
        constexpr uint8_t active = 0x01;
        constexpr uint8_t shutdown = 0x02;
    };
    struct container_list_order
    {
        uint8_t mode;
        std::string query;

        MSGPACK_DEFINE(mode, query)
    };

    inline container_list_order unpack_container_list_order(const std::vector<uint8_t> &content)
    {
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(content.data()), content.size());
        return result.get().as<container_list_order>();
    }

    inline auto mode_value(uint8_t mode) -> std::string
    {

        if (mode == filter::active)
            return "active";
        else if (mode == filter::shutdown)
            return "shutdown";
        return "all";
    }
}

#endif // __DAEMON_DOMAIN_CONTAINERS_ORDERS__