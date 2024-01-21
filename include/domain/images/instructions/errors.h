#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_ERRORS__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_ERRORS__

#include <system_error>
#include <map>
#include <string>

namespace domain::images::instructions
{
    enum class error_code
    {
        file_not_found,
        no_work_directory,
        directory_not_found,
        invalid_origin,
        invalid_destination,
        invalid_copy_instruction,
        no_registry_entries_found,
        no_matching_image_found,
        no_registry_access,
        invalid_order_issued,
        no_mount_points_present
    };
    const inline std::map<error_code, std::string> error_map{
        {error_code::file_not_found, "file not found"},
        {error_code::no_work_directory, "no work directory"},
        {error_code::directory_not_found, "directory not found"},
        {error_code::invalid_origin, "invalid origin"},
        {error_code::invalid_destination, "invalid destination"},
        {error_code::invalid_copy_instruction, "invalid copy instruction"},
        {error_code::no_registry_entries_found, "no registries found"},
        {error_code::no_matching_image_found, "no matching image found"},
        {error_code::no_registry_access, "not authorized to access registry"},
        {error_code::invalid_order_issued, "invalid order issued"},
        {error_code::no_mount_points_present, "no mount points present for this file system"}};

    struct image_failure_category : public std::error_category
    {
        image_failure_category() {}
        virtual ~image_failure_category() = default;
        image_failure_category(const image_failure_category &) = delete;
        const char *name() const noexcept override
        {
            return "image failures";
        }

        std::string message(int ec) const override
        {
            static const std::string unknown_error_code("unknown image failure");
            if (auto pos = error_map.find(static_cast<error_code>(ec)); pos != error_map.end())
            {
                return pos->second;
            }
            return unknown_error_code;
        }
    };

    inline const image_failure_category &__image_failure_category()
    {
        static image_failure_category fc;
        return fc;
    }

    inline const std::error_code make_error_code(error_code ec) noexcept
    {

        return std::error_code{static_cast<int>(ec), __image_failure_category()};
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_ERRORS__