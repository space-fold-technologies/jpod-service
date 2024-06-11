#ifndef __DAEMON_DOMAIN_IMAGES_MAPPINGS__
#define __DAEMON_DOMAIN_IMAGES_MAPPINGS__

#include <string>
#include <vector>
#include <cstdint>
#include <msgpack.hpp>

using namespace std::chrono;

namespace domain::images
{
    struct registry_details
    {
        std::string name;
        std::string uri;
        std::string path;
        MSGPACK_DEFINE(name, uri, path)
    };
    struct authorization_update
    {
        std::string path;
        std::string token;
        MSGPACK_DEFINE(path, token)
    };
    struct registry_access_details
    {
        std::string uri;
        std::string token;
    };

    struct mount_point
    {
        std::string filesystem;
        std::string folder;
        std::string options;
        uint64_t flags;
        MSGPACK_DEFINE(filesystem, folder, options, flags)
    };

    struct image_internals
    {
        std::map<std::string, std::string> labels;
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> env_vars;
        std::vector<mount_point> mount_points;
        MSGPACK_DEFINE(labels, parameters, env_vars, mount_points)
    };

    inline std::vector<uint8_t> pack_image_internals(const image_internals &order)
    {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, order);
        std::vector<uint8_t> output(buffer.size());
        std::memcpy(output.data(), buffer.data(), buffer.size());
        return output;
    }

    inline image_internals unpack_image_internals(const std::vector<uint8_t> &content)
    {
        msgpack::object_handle result;
        msgpack::unpack(result, reinterpret_cast<const char *>(content.data()), content.size());
        return result.get().as<image_internals>();
    }

    struct image_summary_entry
    {
        std::string identifier;
        std::string name;
        std::string repository;
        std::string tag;
        std::size_t size;
        time_point<system_clock, nanoseconds> created_at;
        MSGPACK_DEFINE(identifier, name, repository, tag, size, created_at)
    };

    struct image_summary
    {
        std::vector<image_summary_entry> entries;
        MSGPACK_DEFINE_ARRAY(entries)
    };

    inline std::vector<uint8_t> pack_image_entries(std::vector<image_summary_entry> &entries)
    {
        msgpack::sbuffer buffer;
        msgpack::pack(buffer, image_summary{entries});
        std::vector<uint8_t> output(buffer.size());
        std::memcpy(output.data(), buffer.data(), buffer.size());
        return output;
    }

    struct image_details
    {
        std::string identifier;
        std::string repository;
        std::string registry;
        std::string tag;
        std::string os;
        std::string variant;
        std::string version;
        std::size_t size;
        std::string entry_point;
        std::map<std::string, std::string> labels;
        std::map<std::string, std::string> parameters;
        std::map<uint16_t, std::string> exposed_ports;
        std::map<std::string, std::string> env_vars;
        std::vector<std::string> volumes;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_MAPPINGS__