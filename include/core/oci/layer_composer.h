#ifndef __DAEMON_CORE_OCI_LAYER_COMPOSER__
#define __DAEMON_CORE_OCI_LAYER_COMPOSER__

#include <tl/expected.hpp>
#include <unordered_map>
#include <system_error>
#include <filesystem>
#include <optional>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>
#include <map>

namespace fs = std::filesystem;

namespace core::oci
{
    enum class change_type
    {
        added,
        modified,
        deleted,
        none
    };

    enum class layer_type
    {
        root,
        folder,
        compressed
    };

    struct entry
    {
        fs::path path;
        change_type type;
        fs::file_time_type stamp;
    };

    struct layer_state
    {
        std::optional<fs::path> root_path;
        fs::path target_path;
        fs::path layer_archive;
        std::unordered_map<std::string, entry> changes;
    };

    struct layer_details
    {
        std::string hash_sum;
        fs::path path;
        std::size_t additions;
        std::size_t modifications;
        std::size_t deletions;
    };

    struct configuration_order
    {
        std::string os;
        std::string version;
        std::string arch;
        std::string author;
        std::string variant;
        std::map<std::string, std::string> labels;
        std::map<std::string, std::string> volumes;
        std::map<uint16_t, std::string> ports;
        std::map<std::string, std::string> env_vars;
        std::vector<std::string> entrypoint;
        std::vector<std::string> command;
        std::string work_dir;
        std::map<std::string, fs::path> layers;
        fs::path destination;
        std::string stop_signal;
    };

    struct manifest_order
    {
        std::string os;
        std::string version;
        std::string arch;
        std::string variant;
        std::map<std::string, fs::path> layers;
        fs::path destination;
    };

    using layer_result = tl::expected<layer_state, std::error_code>;
    using layer_result_ptr = tl::expected<std::unique_ptr<layer_state>, std::error_code>;
    using layer_report = tl::expected<layer_details, std::error_code>;
    using configuration_report = tl::expected<fs::path, std::error_code>;
    using manifest_report = tl::expected<fs::path, std::error_code>;
    using hash_report = tl::expected<std::string, std::error_code>;
    [[nodiscard]] layer_result initialize(const fs::path &root_path, const fs::path &target_folder, const fs::path &layer_archive);
    [[nodiscard]] layer_result initialize(const fs::path &target_folder, fs::path layer_archive);
    [[nodiscard]] layer_result copy_root(layer_state state);
    [[nodiscard]] layer_result snapshot_target(layer_state state);
    [[nodiscard]] layer_result diff_to_target(layer_state state);
    [[nodiscard]] layer_report package_layer(layer_state state);
    [[nodiscard]] hash_report note_change(const fs::path &file_path, change_type change);
    [[nodiscard]] configuration_report generate_configuration(const configuration_order &order);
}
#endif // __DAEMON_CORE_OCI_LAYER_COMPOSER__
