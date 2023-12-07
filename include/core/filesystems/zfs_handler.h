#ifndef __JPOD_SERVICE_FILESYSTEMS_FILESYSTEM_ZFS_HANDLER__
#define __JPOD_SERVICE_FILESYSTEMS_FILESYSTEM_ZFS_HANDLER__

#include <core/filesystems/handler.h>
#include <memory>
#include <map>

namespace spdlog
{
    class logger;
}

namespace core::filesystems::zfs
{
    inline std::map<VolumeType, std::string> folders = {
        {VolumeType::CONTAINER, "containers"},
        {VolumeType::STORAGE, "storage"},
        {VolumeType::IMAGE, "images"}};

    class ZFSHandler : public FileSystemHandler, public std::enable_shared_from_this<ZFSHandler>
    {

    public:
        ZFSHandler(const std::string &pool, const std::string &base_folder);
        virtual ~ZFSHandler();
        tl::expected<VolumeDetails, std::error_code> create_volume(
            const std::string &identifier,
            VolumeType type) override;
        tl::expected<VolumeDetails, std::error_code> fetch_volume(
            const std::string &identifier,
            VolumeType type) override;
        tl::expected<VolumeDetails, std::error_code> extract_snapshot(
            const std::string &snapshot_path,
            const std::string &target_name,
            VolumeType type) override;

    private:
        const std::string &pool;
        const std::string &base_folder;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __JPOD_SERVICE_FILESYSTEMS_FILESYSTEM_ZFS_HANDLER__