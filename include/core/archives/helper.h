#ifndef __DAEMON_CORE_ARCHIVES_HELPER__
#define __DAEMON_CORE_ARCHIVES_HELPER__

#include <tl/expected.hpp>
#include <filesystem>
#include <string_view>
#include <memory>
#include <functional>
#include <archive_entry.h>
#include <archive.h>


namespace fs = std::filesystem;
using archive_ptr = std::shared_ptr<archive>;

namespace core::archives
{
    using copy_callback = std::function<void(std::string_view current)>;

    tl::expected<archive_ptr, std::error_code> initialize_reader(const fs::path& archive_path);
    tl::expected<archive_ptr, std::error_code> initialize_writer();
    tl::expected<archive_ptr, std::error_code> archive_writer(const fs::path& archive_path);
    std::error_code add_header(const fs::path& file, archive_entry *entry, archive_ptr &in);
    std::error_code add_entry(const fs::path& file, archive_ptr &in);
    std::error_code copy_entry(archive_ptr &in, archive_ptr &out);
    std::error_code copy_to_destination(archive_ptr &in, archive_ptr &out, const fs::path& destination);
    std::error_code copy_to_destination(archive_ptr &in, archive_ptr &out, const fs::path& destination, copy_callback callback);
}

#endif //__DAEMON_CORE_ARCHIVES_HELPER__