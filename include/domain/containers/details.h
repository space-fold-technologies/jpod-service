#ifndef __DAEMON_DOMAIN_CONTAINERS_DETAILS__
#define __DAEMON_DOMAIN_CONTAINERS_DETAILS__

#include <domain/images/mappings.h>
namespace domain::containers
{
    struct container_details
    {
        std::string identifier;
        std::string name;
        std::string os;
        std::string image_identifier;
        std::map<std::string, std::string> port_map;
        std::map<std::string, std::string> env_vars;
        std::map<std::string, std::string> volumes;
        std::string network_properties;
    };

    struct container_internals
    {
        std::string os;
        std::map<std::string, std::string> port_map;
        std::map<std::string, std::string> env_vars;
        std::map<std::string, std::string> volumes;
        std::string network_properties;
        MSGPACK_DEFINE(os, port_map, env_vars, volumes, network_properties)
    };

    inline container_internals unpack_container_internals(const std::vector<uint8_t> &content)
    {
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(content.data()), content.size());
        return result.get().as<container_internals>();
    }

    inline std::vector<uint8_t> pack_container_internals(const container_internals &order)
    {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, order);
        std::vector<uint8_t> output(buffer.size());
        std::memcpy(output.data(), buffer.data(), buffer.size());
        return output;
    }

    inline void fill_container_details(container_details &details, const container_internals &internals)
    {
        details.os = internals.os;
        details.port_map.insert(internals.port_map.begin(), internals.port_map.end());
        details.env_vars.insert(internals.env_vars.begin(), internals.env_vars.end());
        details.volumes.insert(internals.volumes.begin(), internals.volumes.end());
        details.network_properties = internals.network_properties;
    }

    struct container_properties
    {
        std::string identifier;
        std::string os;
        std::string name;
        std::string image_identifier;
        std::map<std::string, std::string> port_map;
        std::map<std::string, std::string> env_vars;
        std::map<std::string, std::string> volumes;
        std::string network_properties;
    };

    inline void fill_container_properties(container_properties &properties, const container_internals &internals)
    {
        properties.os = internals.os;
        properties.port_map.insert(internals.port_map.begin(), internals.port_map.end());
        properties.env_vars.insert(internals.env_vars.begin(), internals.env_vars.end());
        properties.volumes.insert(internals.volumes.begin(), internals.volumes.end());
        properties.network_properties = internals.network_properties;
    }

    struct container_summary_entry
    {
        std::string identifier;
        std::string name;
        std::string image;
        std::string status;
        std::map<std::string, std::string> port_map;
        time_point<system_clock, nanoseconds> created_at;
        MSGPACK_DEFINE(identifier, name, image, status, port_map, created_at)
    };

    struct container_summary
    {
        std::vector<container_summary_entry> entries;

        MSGPACK_DEFINE(entries)
    };

    inline std::vector<uint8_t> pack_container_summary(const std::vector<container_summary_entry> &entries)
    {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, container_summary{entries});
        std::vector<uint8_t> output(buffer.size());
        std::memcpy(output.data(), buffer.data(), buffer.size());
        return output;
    }

    struct volume_details
    {
        std::string identifier;
        std::string container_identifier;
        std::string driver; // default local {zfs|nullfs}
        std::string filesystem;
        std::string path;
        std::string source;
        std::string options;
        uint64_t flags;
    };

    struct volume_entry
    {
        std::string driver; // default local {zfs|nullfs}
        std::string filesystem;
        std::string path;
        std::string source;
        std::string options;
        uint64_t flags;
    }; 
}
#endif // __DAEMON_DOMAIN_CONTAINERS_DETAILS__