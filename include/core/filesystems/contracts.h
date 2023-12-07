#ifndef __JPOD_SERVICE_FILESYSTEMS_FILESYSTEM_CONTRACTS__
#define __JPOD_SERVICE_FILESYSTEMS_FILESYSTEM_CONTRACTS__
#include <string>
#include <filesystem>

namespace core::filesystems
{
    enum class VolumeType : uint8_t
    {
        CONTAINER = 0x00,
        IMAGE = 0x01,
        STORAGE = 0x02
    };
    struct VolumeDetails
    {
        std::string name;
        std::string virtual_path;
        std::string absolute_path;
        std::size_t used;
        std::size_t available;
        std::size_t refer;
        VolumeType type;
    };
    inline auto volume_details(
        std::string name,
        std::string virtual_path,
        std::string absolute_path,
        std::size_t used,
        std::size_t available,
        std::size_t refer,
        VolumeType type) -> VolumeDetails
    {
        return VolumeDetails{
            name,
            virtual_path,
            absolute_path,
            used,
            available,
            refer,
            type};
    }
    struct SnapshotDetails
    {
        std::string identifier;
        std::string path;
        std::size_t used;
        VolumeType type;
    };
    inline auto snapshot_details(
        std::string name,
        std::string path,
        std::size_t used,
        VolumeType type) -> SnapshotDetails
    {
        return SnapshotDetails{
            name,
            path,
            used,
            type};
    };
    struct ImageDetails
    {
    };
    struct ArchiveDetails
    {
        std::string identifier;
        std::filesystem::path path;
        std::size_t size;
    };
}
#endif // __JPOD_SERVICE_FILESYSTEMS_FILESYSTEM_CONTRACTS__