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
}

#endif //__DAEMON_CORE_ARCHIVES_HELPER__