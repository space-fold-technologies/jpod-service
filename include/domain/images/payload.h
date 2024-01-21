#ifndef __DAEMON_DOMAIN_IMAGES_PAYLOADS__
#define __DAEMON_DOMAIN_IMAGES_PAYLOADS__

#include <msgpack/msgpack.hpp>

using namespace std::chrono;

namespace domain::images
{
    struct list_order
    {
        std::string query;
        template <class T>
        void pack(T &pack)
        {
            pack(query);
        }
    };

    inline list_order unpack_list_order(const std::vector<uint8_t> &content)
    {
        return msgpack::unpack<list_order>(content);
    }

    struct summary_entry
    {
        std::string identifier;
        std::string repository;
        std::string tag;
        std::size_t size;
        std::string query;
        time_point<system_clock, nanoseconds> created_at;
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
}
#endif // __DAEMON_DOMAIN_IMAGES_PAYLOADS__