#ifndef __JPOD_SERVICE_FILESYSTEMS_FILESYSTEM_OPERATOR__
#define __JPOD_SERVICE_FILESYSTEMS_FILESYSTEM_OPERATOR__

#include <string>
#include <tl/expected.hpp>
#include <filesystem>
#include <system_error>
#include <core/filesystems/contracts.h>

namespace fs = std::filesystem;
namespace core::filesystems
{
    class FileSystemHandler
    {
    public:
        virtual tl::expected<VolumeDetails, std::error_code> create_volume(
            const std::string &identifier,
            VolumeType type) = 0;
        virtual tl::expected<VolumeDetails, std::error_code> fetch_volume(const std::string &identifier, VolumeType type) = 0;
        virtual tl::expected<VolumeDetails, std::error_code> extract_snapshot(
            const std::string &snapshot_path,
            const std::string &target_name,
            VolumeType type) = 0;
    };
}

#endif // __JPOD_SERVICE_FILESYSTEMS_FILESYSTEM_OPERATOR__