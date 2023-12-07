#include <core/networks/http/file_destination.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace core::networks::http
{
    FileDestination::FileDestination(const std::string file_path, std::size_t buffer_size) : file_path(file_path), buffer_size(buffer_size), logger(spdlog::get("jpod"))
    {
    }
    bool FileDestination::is_valid()
    {
        if (!fs::exists(file_path))
        {
            auto path = fs::path(file_path);
            fs::create_directories(path.parent_path());
            fs::permissions(path.parent_path(),
                            std::filesystem::perms::owner_all | std::filesystem::perms::group_all,
                            std::filesystem::perm_options::add);
            return true;
        }
        if (!fs::is_regular_file(file_path))
        {
            return false;
        }
        auto permissions = fs::status(file_path).permissions();
        return fs::perms::none != (fs::perms::owner_write & permissions) || fs::perms::none != (fs::perms::group_write & permissions);
    }
    size_t FileDestination::chunk_size() const
    {
        return buffer_size;
    }
    size_t FileDestination::write(const std::vector<uint8_t> &data)
    {
        logger->info("writing {} bytes to file", data.size());
        std::ofstream file(file_path, std::ios::binary | std::ios::app);
        file.write(reinterpret_cast<const char *>(data.data()), data.size());
        file.close();
        return data.size();
    }
    FileDestination::~FileDestination()
    {
    }
}