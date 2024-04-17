#ifndef __DAEMON_DOMAIN_NETWORKING_PAYLOADS__
#define __DAEMON_DOMAIN_NETWORKING_PAYLOADS__

#include <msgpack.hpp>

namespace domain::networking
{
    struct network_creation_order
    {
        std::string name;
        std::string subnet;
        std::string scope;
        MSGPACK_DEFINE(name, subnet, scope)
    };

    inline network_creation_order unpack_network_creation_order(const std::vector<uint8_t> &content)
    {
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(content.data()), content.size());
        return result.get().as<network_creation_order>();
    }

    struct network_term_order
    {
        std::string name;
        MSGPACK_DEFINE(name)
    };

    inline network_term_order unpack_network_term_order(const std::vector<uint8_t> &content)
    {
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(content.data()), content.size());
        return result.get().as<network_term_order>();
    }

    struct network_details
    {
        std::string name;
        std::string driver;
        std::string subnet;
        std::string scope;
        std::string status;
        MSGPACK_DEFINE(name, driver, subnet, scope, status)
    };

    struct network_list
    {
        std::vector<network_details> networks;
        MSGPACK_DEFINE(networks)
    };

    inline std::vector<uint8_t> pack_network_list(network_list &order)
    {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, order);
        std::vector<uint8_t> output(buffer.size());
        std::memcpy(output.data(), buffer.data(), buffer.size());
        return output;
    }
}
#endif // __DAEMON_DOMAIN_NETWORKING_PAYLOADS__