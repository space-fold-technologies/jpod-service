#ifndef __JPOD_SERVICE_IMAGES_DATA_TYPES__
#define __JPOD_SERVICE_IMAGES_DATA_TYPES__

#include <msgpack/msgpack.hpp>
#include <vector>
#include <string>
#include <map>
#include <chrono>

namespace images
{
    enum class StepType
    {
        FROM,
        RUN,
        WORKDIR,
        COPY
    };

    struct MountPoint
    {
        std::string filesystem;
        std::string folder;
        std::string options;
        uint64_t flags;
    };

    struct Stage
    {
        std::string image_name;
        std::string image_version;
        std::vector<std::string> labels;
        std::map<std::string, std::string> instructions;
    };

    struct ConstructionDetails
    {
        std::string name;
        std::string tag;
        std::string current_directory;
        std::vector<Stage> stages;
        std::map<std::string, std::string> env_vars;
        std::string entry_point;
    };

    struct ImageDetails
    {
        std::string id;
        std::string host;
        std::string os;
        std::string version;
        std::string variant;
        std::string tag;
        std::string created;
        std::string repository;
        std::size_t size;
        bool local;
        std::string filesystem_identifier;
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> env_vars;
    };

    struct AccessDetails
    {
        std::string token;
        std::string uri;
    };
    struct RegistryAccessDetails
    {
        std::string username;
        std::string passord;
    };
    struct ImageSummary
    {
        std::string id;
        std::string host;
        std::string os;
        std::string version;
        std::string variant;
        std::string tag;
        std::string created;
        std::string repository;
        std::size_t size;

        template <class T>
        void pack(T &pack)
        {
            pack(id, host, os, version, variant, tag, created, repository, size);
        }
    };

    struct ImageQuery
    {
        std::string name;
        std::string version;
        std::string architecture;
        template <class T>
        void pack(T &pack)
        {
            pack(name, version, architecture);
        }
    };
    struct ImageMetaData
    {
        std::string id;
        std::string name;
        std::string version;
        std::size_t size;
        template <class T>
        void pack(T &pack)
        {
            pack(id);
        }
    };

    struct BuildUpdate
    {
        std::string id;
        std::string type;
        std::vector<uint8_t> data;
        template <class T>
        void pack(T &pack)
        {
            pack(id, type, data);
        }
    };
    struct Progress
    {
        std::string target;
        std::size_t start;
        std::size_t end;
        std::size_t total;
        std::string unit;
        bool complete;
        template <class T>
        void pack(T &pack)
        {
            pack(target, start, end, total, unit, complete);
        }
    };
}

#endif // __JPOD_SERVICE_IMAGES_DATA_TYPES__