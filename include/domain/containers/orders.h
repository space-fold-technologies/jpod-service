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
    struct container_remove_order
    {
        std::string term;
        bool force;
        MSGPACK_DEFINE(term, force)
    };
    inline container_remove_order unpack_container_remove_order(const std::vector<uint8_t> &content)
    {
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(content.data()), content.size());
        return result.get().as<container_remove_order>();
    }

    enum class shell_order_type
    {
        start_session,
        terminal_feed,
        terminal_size
    };

    struct container_shell_order
    {
        shell_order_type type;
        std::vector<uint8_t> data;

        MSGPACK_DEFINE(type, data)
    };

    inline container_shell_order unpack_container_shell_order(const std::vector<uint8_t> &content)
    {
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(content.data()), content.size());
        return result.get().as<container_shell_order>();
    }

    enum class filter_mode
    {
        active,
        all,
        shutdown
    };

    struct container_list_order
    {
        filter_mode mode;
        std::string query;

        MSGPACK_DEFINE(mode, query)
    };

    inline container_list_order unpack_container_list_order(const std::vector<uint8_t> &content)
    {
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(content.data()), content.size());
        return result.get().as<container_list_order>();
    }

    inline auto mode_value(filter_mode mode) -> std::string
    {

        if (mode == filter_mode::active)
            return "active";
        else if (mode == filter_mode::shutdown)
            return "shutdown";
        return "all";
    }
}
MSGPACK_ADD_ENUM(domain::containers::shell_order_type);
MSGPACK_ADD_ENUM(domain::containers::filter_mode);
#endif // __DAEMON_DOMAIN_CONTAINERS_ORDERS__