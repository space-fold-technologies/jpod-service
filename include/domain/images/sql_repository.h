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
        std::optional<registry> fetch_registry_by_uri(const std::string &path) override;
        std::optional<registry> fetch_registry_by_name(const std::string &name) override;
        bool has_image(const std::string &registry, const std::string &name, const std::string& tag) override;
        std::error_code save_image_details(const image_details &details) override;
        std::optional<image_details> fetch_image_details(const std::string &registry, const std::string &name, const std::string &tag) override;
        std::optional<std::string> fetch_image_identifier(const std::string &registry, const std::string &name, const std::string& tag) override;
        std::vector<mount_point> fetch_image_mount_points(const std::string &registry, const std::string &name, const std::string& tag) override;
    private:
        core::sql::pool::data_source &data_source;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_SQL_REPOSITORY__