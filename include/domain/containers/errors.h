#ifndef __DAEMON_DOMAIN_CONTAINERS_ERRORS__
#define __DAEMON_DOMAIN_CONTAINERS_ERRORS__

#include <string>
#include <map>

namespace domain::containers
{
    enum class container_error
    {
        exists,
        unknown_image,
        extraction_failed,
        not_found,
        busy,
        invalid_image
    };
    inline std::map<container_error, std::string> container_error_map =
    {
            {container_error::exists, "a container with the specified identifier already exists"},
            {container_error::unknown_image, "the image specified to create the container does not exist"},
            {container_error::extraction_failed, "image extraction failed during container creations"},
            {container_error::not_found, "no matching container was found matching"},
            {container_error::busy, "the specified container is busy at the moment"},
            {container_error::invalid_image, "invalid image specified, please re-check the repository and tag"}
    };
    struct container_failure_category : public std::error_category
    {
        container_failure_category() {}
        virtual ~container_failure_category() = default;
        container_failure_category(const container_failure_category &) = delete;
        const char *name() const noexcept override
        {
            return "container failures";
        }

        std::string message(int ec) const override
        {
            static const std::string unknown_error_code("unknown container failure");
            if (auto pos = container_error_map.find(static_cast<container_error>(ec)); pos != container_error_map.end())
            {
                return pos->second;
            }
            return unknown_error_code;
        }
    };

    inline const container_failure_category &__container_failure_category()
    {
        static container_failure_category fc;
        return fc;
    }

    inline const std::error_code make_container_failure(container_error ec) noexcept
    {

        return std::error_code{static_cast<int>(ec), __container_failure_category()};
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_ERRORS__