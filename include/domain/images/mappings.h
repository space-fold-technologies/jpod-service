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
        std::string authorization_type;
        std::string authorization_url;
    };

    struct mount_point
    {
        std::string filesystem;
        std::string folder;
        std::string options;
        uint64_t flags;
        MSGPACK_DEFINE(filesystem, folder, options, flags)
    };

    struct image_summary_entry
    {
        std::string identifier;
        std::string repository;
        std::string registry;
        std::string tag;
        std::size_t size;
        time_point<system_clock, nanoseconds> created_at;
        MSGPACK_DEFINE(identifier, repository, registry, tag, size, created_at)
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
        std::string tag_reference;
        std::string os;
        std::string variant;
        std::string version;
        std::size_t size;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_MAPPINGS__