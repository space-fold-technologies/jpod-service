#ifndef __DAEMON_DOMAIN_CONTAINERS_ORDERS__
#define __DAEMON_DOMAIN_CONTAINERS_ORDERS__

#include <msgpack/msgpack.hpp>

namespace domain::containers
{
    struct container_creation_order
    {
        std::string tagged_image;
        std::string name;
        std::map<std::string, std::string> port_map;
        std::map<std::string, std::string> env_vars;
        std::string network_properties;

        template <class T>
        void pack(T &pack)
        {
            pack(tagged_image, name, port_map, env_vars);
        }
    };

    inline container_creation_order unpack_container_creation_order(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<container_creation_order>(content);
    }

    struct container_term_order
    {
        std::string term;
        template <class T>
        void pack(T &pack)
        {
            pack(term);
        }
    };

    inline container_term_order unpack_container_term_order(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<container_term_order>(content);
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
        template <class T>
        void pack(T &pack)
        {
            pack(type, data);
        }
    };

    inline container_shell_order unpack_container_shell_order(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<container_shell_order>(content);
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
        template <class T>
        void pack(T &pack)
        {
            pack(mode, query);
        }
    };

    inline container_list_order unpack_container_list_order(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<container_list_order>(content);
    }

    inline auto mode_value(uint8_t mode) -> std::string
    {
        if (mode == filter::all)
            return "all";
        if (mode == filter::active)
            return "active";
        if (mode == filter::shutdown)
            return "shutdown";
    }
}

#endif // __DAEMON_DOMAIN_CONTAINERS_ORDERS__