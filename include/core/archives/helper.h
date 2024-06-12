#ifndef __DAEMON_CORE_ARCHIVES_HELPER__
#define __DAEMON_CORE_ARCHIVES_HELPER__

#include <tl/expected.hpp>
#include <filesystem>
#include <memory>
#include <archive.h>
#include <archive_entry.h>

namespace fs = std::filesystem;
using archive_ptr = std::shared_ptr<archive>;
namespace core::archives
{

    tl::expected<archive_ptr, std::error_code> initialize_reader(fs::path archive_path);
    tl::expected<archive_ptr, std::error_code> initialize_writer();
    std::error_code copy_entry(struct archive *in, struct archive *out);
    std::error_code copy_to_destination(archive_ptr in, archive_ptr out, fs::path& destination);
}

#endif //__DAEMON_CORE_ARCHIVES_HELPER__