#ifndef __JPOD_SERVICE_IMAGES_IMAGE_BUILDER__
#define __JPOD_SERVICE_IMAGES_IMAGE_BUILDER__

#include <asio/io_context.hpp>
#include <images/data_types.h>
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

namespace images
{
    class ImageBuilder
    {
    public:
        ImageBuilder(
            asio::io_context &context,
            std::shared_ptr<FileSystemHandler> filesystem_handler);
        virtual ~ImageBuilder();
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

#endif // __JPOD_SERVICE_IMAGES_IMAGE_BUILDER__