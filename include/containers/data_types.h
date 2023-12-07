#ifndef __JPOD_SERVICE_CONTAINERS_DATA_TYPES__
#define __JPOD_SERVICE_CONTAINERS_DATA_TYPES__

#include <msgpack/msgpack.hpp>
#include <vector>
#include <string>
#include <map>

namespace containers
{
    struct Address
    {
        std::string type;
        std::string address;
    };

    struct MountPoint
    {
        std::string filesystem;
        std::string folder;
        std::string options;
        uint64_t flags;
    };

    struct Properties
    {
        std::string snapshot_path;
        std::vector<MountPoint> mount_points;
    };

    struct ContainerDetails
    {
        std::string id;
        std::string host;
        std::string os;
        std::string version;
        std::string variant;
        std::string path;
        std::string entry_point;
        std::string username;
        std::vector<Address> addresses;
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> volumes;
        std::map<std::string, std::string> env_vars;
        std::vector<MountPoint> mount_points;
    };
    struct ContainerSummary
    {
        std::string id;
        std::string name;
        std::string image;
        std::string entry_point;
        std::string status;
        std::string created;
        std::vector<std::string> ports;

        template <class T>
        void pack(T &pack)
        {
            pack(id, name, image, entry_point, status, created, ports);
        }
    };
}

#endif // __JPOD_SERVICE_CONTAINERS_DATA_TYPES__

// https://www.youtube.com/watch?v=gnvDPCXktWQ