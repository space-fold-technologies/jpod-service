#ifndef __DAEMON_DOMAIN_IMAGES_MAPPINGS__
#define __DAEMON_DOMAIN_IMAGES_MAPPINGS__

#include <string>
#include <vector>
#include <cstdint>
#include <msgpack/msgpack.hpp>

namespace domain::images
{
    struct registry
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
        template <class T>
        void pack(T &pack)
        {
            pack(filesystem, folder, options, flags);
        }
    };

    struct image_internals
    {
        std::map<std::string, std::string> labels;
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> env_vars;
        std::vector<mount_point> mount_points;
        template <class T>
        void pack(T &pack)
        {
            pack(labels, parameters, env_vars, mount_points);
        }
    };

    inline std::vector<uint8_t> pack_image_internals(image_internals &order)
    {
        return msgpack::pack(order);
    }

    inline image_internals unpack_image_internals(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<image_internals>(content);
    }

    struct image_details
    {
        std::string identifier;
        std::string name;
        std::string tag;
        std::string os;
        std::string variant;
        std::string version;
        std::size_t size;
        std::string entry_point;
        std::string registry_uri;
        std::map<std::string, std::string> labels;
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> env_vars;
        std::vector<mount_point> mount_points;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_MAPPINGS__