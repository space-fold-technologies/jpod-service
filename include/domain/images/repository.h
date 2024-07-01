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
        virtual std::error_code add_registry(const registry_details &details) = 0;
        virtual std::error_code update_token(const authorization_update &update) = 0;
        virtual std::optional<registry_access_details> fetch_registry_by_path(const std::string &path) = 0;
        virtual std::optional<registry_access_details> fetch_registry_by_name(const std::string &name) = 0;
        virtual bool has_image(const std::string &registry, const std::string &name, const std::string &tag) = 0;
        virtual std::error_code save_image_details(const image_details &details) = 0;
        virtual std::optional<image_details> fetch_image_details(const std::string &registry, const std::string &name, const std::string &tag) = 0;
        virtual std::vector<image_summary_entry> fetch_matching_details(const std::string &query) = 0;
        virtual std::optional<std::string> fetch_image_identifier(const std::string &registry, const std::string &name, const std::string &tag) = 0;
        virtual bool has_containers(const std::string &query) = 0;
        virtual std::error_code remove(const std::string &query) = 0;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_REPOSITORY__