#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_ERRORS__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_ERRORS__

#include <system_error>
#include <map>
#include <string>

namespace domain::images
{
    enum class error_code
    {
        already_exists,
        invalid_order,
        unknown_image,
        unknown_registry
    };
    const inline std::map<error_code, std::string> error_map{
        {error_code::already_exists, "the specified image already exists"},
        {error_code::invalid_order, "invalid image order"},
        {error_code::unknown_image, "the image specified is unknown"},
        {error_code::unknown_registry, "unknown registry specified"}};

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