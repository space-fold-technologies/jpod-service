#ifndef __JPOD_SERVICE_CONTAINERS_CONTAINER_BUILDER__
#define __JPOD_SERVICE_CONTAINERS_CONTAINER_BUILDER__

#include <asio/io_context.hpp>
#include <containers/data_types.h>
#include <map>

namespace spdlog
{
    class logger;
}

namespace core::filesystems
{
    class FileSystemHandler;
}

using namespace core::filesystems;
namespace containers
{
    class ContainerBuilder
    {

    public:
        ContainerBuilder(
            asio::io_context &context,
            std::shared_ptr<FileSystemHandler> filesystem_handler);
        virtual ~ContainerBuilder();
        void build(const Properties &properties);

    private:
        std::error_code mount_filesystems(const std::string &parent_folder, const std::vector<MountPoint> &mount_points);

    private:
        asio::io_context &context;
        std::string current_directory;
        std::shared_ptr<FileSystemHandler> filesystem_handler;
        std::shared_ptr<spdlog::logger> logger;
        std::map<std::string, uint64_t> mounted_points;
        std::string id;
        std::string filesystem_identifier;
    };
}
#endif // __JPOD_SERVICE_CONTAINERS_CONTAINER_BUILDER__