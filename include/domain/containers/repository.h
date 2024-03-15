#ifndef __DAEMON_DOMAIN_CONTAINERS_REPOSITORY__
#define __DAEMON_DOMAIN_CONTAINERS_REPOSITORY__

#include <domain/containers/details.h>
#include <optional>
#include <system_error>

namespace domain::containers
{
    class container_repository
    {
    public:
        virtual std::optional<domain::images::image_details> fetch_image_details(const std::string &registry, const std::string &name, const std::string &tag) = 0;
        virtual std::optional<container_details> fetch(const std::string &identifier) = 0;
        virtual std::optional<container_details> first_match(const std::string &query) = 0;
        virtual std::optional<std::string> first_identifier_match(const std::string &query) = 0;
        virtual std::error_code save(const container_properties &properties) = 0;
        virtual std::vector<container_summary_entry> fetch_match(const std::string &query, const std::string &status) = 0;
        virtual bool is_running(const std::string &query) = 0;
        virtual bool exists(const std::string &query) = 0;
        virtual std::error_code register_status(const std::string &identifier, const std::string &status) = 0;
        virtual std::error_code remove(const std::string &query) = 0;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_REPOSITORY__