#ifndef __DAEMON_DOMAIN_IMAGES_SQL_REPOSITORY__
#define __DAEMON_DOMAIN_IMAGES_SQL_REPOSITORY__
#include <domain/images/repository.h>

namespace core::sql::pool
{
    class data_source;
}
namespace domain::images
{
    class sql_image_repository : public image_repository
    {
    public:
        sql_image_repository(core::sql::pool::data_source &data_source);
        virtual ~sql_image_repository();
        std::error_code add_registry(const registry_details &details) override;
        std::error_code update_token(const authorization_update &update) override;
        std::optional<registry_access_details> fetch_registry_by_path(const std::string &path) override;
        std::optional<registry_access_details> fetch_registry_by_name(const std::string &name) override;
        bool has_image(const std::string &registry, const std::string &name, const std::string &tag) override;
        std::error_code save_image_details(const image_details &details) override;
        std::optional<image_details> fetch_image_details(const std::string &registry, const std::string &name, const std::string &tag) override;
        std::vector<image_summary_entry> fetch_matching_details(const std::string &query) override;
        std::optional<std::string> fetch_image_identifier(const std::string &registry, const std::string &name, const std::string &tag) override;
        bool has_containers(const std::string &query) override;
        std::error_code remove(const std::string &query) override;

    private:
        core::sql::pool::data_source &data_source;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_SQL_REPOSITORY__