#ifndef __DAEMON_DOMAIN_IMAGES_PAYLOADS__
#define __DAEMON_DOMAIN_IMAGES_PAYLOADS__

#include <msgpack/msgpack.hpp>
#include <yaml-cpp/yaml.h>

using namespace std::chrono;

namespace domain::images
{
    struct image_term_order
    {
        std::string term;
        template <class T>
        void pack(T &pack)
        {
            pack(term);
        }
    };

    inline image_term_order unpack_image_term_order(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<image_term_order>(content);
    }

    struct summary_entry
    {
        std::string identifier;
        std::string repository;
        std::string tag;
        std::size_t size;
        time_point<system_clock, milliseconds> created_at;
        template <class T>
        void pack(T &pack)
        {
            pack(identifier, repository, tag, size);
        }
    };

    struct summary
    {
        std::string name;
        std::vector<summary_entry> entries;
        template <class T>
        void pack(T &pack)
        {
            pack(name, entries);
        }
    };

    inline std::vector<uint8_t> pack_summary(summary &order)
    {
        return msgpack::pack(order);
    }

    inline summary unpack_summary(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<summary>(content);
    }

    enum class step_type : int
    {
        from = 0,
        run = 1,
        work_dir = 2,
        copy = 3
    };

    struct stage
    {
        std::string name;
        std::string tag;
        std::map<std::string, std::string> labels;
        std::map<std::string, int> steps;

        template <class T>
        void pack(T &pack)
        {
            pack(name, tag, labels, steps);
        }

        bool operator==(const stage rhs)
        {
            return (this->name == rhs.name && this->tag == rhs.tag && this->labels == rhs.labels && this->steps == rhs.steps);
        }
    };

    struct build_order
    {
        std::string name;
        std::string tag;
        std::string current_directory;
        std::vector<stage> stages;
        std::string entry_point;

        template <class T>
        void pack(T &pack)
        {
            pack(name, tag, current_directory, stages, entry_point);
        }
    };

    inline build_order unpack_build_order(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<build_order>(content);
    }

    struct image_query
    {
        std::string name;
        std::string tag;
        std::string version;
        std::string architecture;

        template <class T>
        void pack(T &pack)
        {
            pack(name, tag, version, architecture);
        }
    };

    inline std::vector<uint8_t> pack_image_query(image_query &order)
    {
        return msgpack::pack(order);
    }

    struct mount_point_details
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

    struct image_meta
    {
        std::string identifier;
        std::string host;
        std::string name;
        std::string tag;
        std::string os;
        std::string variant;
        std::string version;
        std::size_t size;
        std::string repository;
        std::map<std::string, std::string> env_vars;
        std::map<std::string, std::string> parameters;
        std::vector<mount_point_details> mount_points;
        template <class T>
        void pack(T &pack)
        {
            pack(identifier, host, name, tag, os, variant, version, size, env_vars, parameters, mount_points);
        }
    };

    inline image_meta unpack_image_details(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<image_meta>(content);
    }

    struct progress_frame
    {
        std::string entry_name;
        std::string sub_entry_name;
        std::string feed;
        double percentage;

        template <class T>
        void pack(T &pack)
        {
            pack(entry_name, sub_entry_name, percentage);
        }
    };

    inline std::vector<uint8_t> pack_progress_frame(progress_frame &order)
    {
        return msgpack::pack(order);
    }

    struct import_details
    {
        std::string name;
        std::string tag;
        std::string os;
        std::string variant;
        std::string version;
        std::string entry_point;
        std::map<std::string, std::string> labels;
        std::map<std::string, std::string> env_vars;
        std::map<std::string, std::string> parameters;
        std::vector<mount_point_details> mount_points;
    };

    inline import_details unpack_import_details(const std::vector<uint8_t> &content)
    {
        import_details details;
        YAML::Node parsed_content = YAML::Load(std::string(content.begin(), content.end()));
        details.name = parsed_content["name"].as<std::string>();
        details.tag = parsed_content["tag"].as<std::string>();
        details.os = parsed_content["os"].as<std::string>();
        details.entry_point = parsed_content["entry_point"].as<std::string>();
        details.variant = parsed_content["variant"].as<std::string>();
        details.version = parsed_content["version"].as<std::string>();
        details.labels = parsed_content["labels"].as<std::map<std::string, std::string>>();
        if (parsed_content["environment"])
        {
            details.env_vars = parsed_content["environment"].as<std::map<std::string, std::string>>();
        }
        for (const auto &node : parsed_content["parameters"])
        {
            details.parameters.emplace(node.first.as<std::string>(), node.second.as<std::string>(" "));
        }
        for (const auto &node : parsed_content["mount_points"])
        {
            details.mount_points.push_back(mount_point_details{
                node["filesystem"].as<std::string>(),
                node["folder"].as<std::string>(),
                node["options"].as<std::string>(),
                node["flags"].as<uint64_t>()});
        }
        return details;
    }

    struct import_order
    {

        std::string archive_path;

        template <class T>
        void pack(T &pack)
        {
            pack(archive_path);
        }
    };

    inline import_order unpack_import_order(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<import_order>(content);
    }
}
#endif // __DAEMON_DOMAIN_IMAGES_PAYLOADS__