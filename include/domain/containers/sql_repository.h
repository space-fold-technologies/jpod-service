#ifndef __DAEMON_DOMAIN_CONTAINERS_SQL_REPOSITORY__
#define __DAEMON_DOMAIN_CONTAINERS_SQL_REPOSITORY__

#include <domain/containers/repository.h>

namespace core::sql::pool
{
    class data_source;
}

namespace domain::containers
{
    class sql_container_repository : public container_repository
    {
    public:
        sql_container_repository(core::sql::pool::data_source &data_source);
        virtual ~sql_container_repository();
        std::optional<domain::images::image_details> fetch_image_details(const std::string &registry, const std::string &name, const std::string &tag) override;
        std::optional<container_details> fetch(const std::string &identifier) override;
        std::optional<container_details> first_match(const std::string &query) override;
        std::optional<std::string> first_identifier_match(const std::string &query) override;
        std::error_code save(const container_properties &properties) override;

    private:
        core::sql::pool::data_source &data_source;
    };
}

#endif // __DAEMON_DOMAIN_CONTAINERS_SQL_REPOSITORY__