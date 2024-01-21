#ifndef __DAEMON_DOMAIN_IMAGES_REPOSITORY__
#define __DAEMON_DOMAIN_IMAGES_REPOSITORY__

#include <domain/images/mappings.h>
#include <vector>
#include <optional>
#include <system_error>

namespace domain::images
{
    class image_repository
    {
    public:
        virtual ~image_repository() = default;
        virtual std::optional<registry> fetch_registry_by_uri(const std::string &path) = 0;
        virtual std::optional<registry> fetch_registry_by_name(const std::string &name) = 0;
        virtual bool has_image(const std::string &registry, const std::string &name, const std::string &tag) = 0;
        virtual std::error_code save_image_details(const image_details &details) = 0;
        virtual std::optional<image_details> fetch_image_details(const std::string &registry, const std::string &name, const std::string &tag) = 0;
        virtual std::optional<std::string> fetch_image_identifier(const std::string &registry, const std::string &name, const std::string &tag) = 0;
        virtual std::vector<mount_point> fetch_image_mount_points(const std::string &registry, const std::string &name, const std::string &tag) = 0;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_REPOSITORY__