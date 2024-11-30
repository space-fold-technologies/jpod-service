#ifndef __DAEMON_CORE_OCI_MANIFEST_COMPOSER__
#define __DAEMON_CORE_OCI_MANIFEST_COMPOSER__

#include <tl/expected.hpp>
#include <system_error>
#include <filesystem>
#include <string>
#include <vector>
#include <map>

namespace fs = std::filesystem;

namespace core::oci
{
    struct image_entry_details
    {
        std::string architecture;
        std::string os;
        std::string variant;
        fs::path config_path;
        std::vector<fs::path> layers;
        std::map<std::string, std::string> annotations;
        fs::path destination;
    };

    struct manifest_report
    {
        std::string hash;
        std::size_t size;
        fs::path path;
    };

    using manifest_result = tl::expected<manifest_report, std::error_code>;
    using registration_result = tl::expected<std::string, std::error_code>;
    [[nodiscard]] manifest_result create_image_manifest(const image_entry_details &details);
    [[nodiscard]] registration_result register_manifest(const manifest_report);
}

#endif // __DAEMON_CORE_OCI_MANIFEST_COMPOSER__
