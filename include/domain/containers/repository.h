#ifndef __DAEMON_DOMAIN_CONTAINERS_REPOSITORY__
#define __DAEMON_DOMAIN_CONTAINERS_REPOSITORY__

#include <domain/containers/details.h>
#include <tl/expected.hpp>
#include <system_error>
#include <optional>

namespace domain::containers
{
    using volumes = std::vector<volume_entry>;
    class container_repository
    {
    public:
        virtual std::optional<std::string> fetch_image_identifier(const std::string &registry, const std::string &name, const std::string &tag) = 0;
        virtual std::optional<domain::images::image_details> fetch_image_details(const std::string& identifier) = 0;
        virtual std::optional<container_details> fetch(const std::string &identifier) = 0;
        virtual std::optional<container_details> first_match(const std::string &query) = 0;
        virtual std::optional<std::string> first_identifier_match(const std::string &query) = 0;
        virtual std::error_code save(const container_properties &properties) = 0;
        virtual std::vector<container_summary_entry> fetch_match(const std::string &query, const std::string &status) = 0;
        virtual bool is_running(const std::string &query) = 0;
        virtual bool exists(const std::string &query) = 0;
        virtual std::error_code register_status(const std::string &identifier, const std::string &status) = 0;
        virtual std::error_code remove(const std::string &query) = 0;
        virtual std::error_code add_entry(const volume_details &entry) = 0;
        virtual tl::expected<volumes, std::error_code> fetch_volumes(const std::string &identifier) = 0;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_REPOSITORY__