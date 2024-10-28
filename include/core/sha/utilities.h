#ifndef __DAEMON_CORE_SHA_UTILITIES__
#define __DAEMON_CORE_SHA_UTILITIES__

#include <tl/expected.hpp>
#include <system_error>
#include <filesystem>
#include <cstdint>
#include <string>
#include <vector>

 
namespace core::sha
{
    using hash_report = tl::expected<std::string, std::error_code>;
    hash_report compute_sha256_of_file(const std::filesystem::path &path);
    hash_report compute_sha256_sum(const std::vector<std::string>& hash_list);
}

#endif //__DAEMON_CORE_SHA_UTILITIES__