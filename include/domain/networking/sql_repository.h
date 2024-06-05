#ifndef __DAEMON_DOMAIN_NETWORKING_SQL_REPOSITORY__
#define __DAEMON_DOMAIN_NETWORKING_SQL_REPOSITORY__

#include <domain/networking/repository.h>

namespace core::sql::pool
{
    class data_source;
}

namespace domain::networking
{
    class sql_network_repository : public network_repository
    {
    public:
        sql_network_repository(core::sql::pool::data_source &data_source);
        virtual ~sql_network_repository();
        bool is_known(const std::string &name) override;
        std::error_code add(const network_entry &entry) override;
        std::error_code update_status(const std::string &name, const std::string &status) override;
        bool is_a_member(const std::string &network_name, const std::string &container_identifier) override;
        std::error_code join(const std::string &code, const std::string& members, const std::string &container_identifier) override;
        std::optional<network_membership> members(const std::string &network_name, const std::string &container_identifier) override;
        std::error_code leave(const std::string &network_name, const std::string &container_identifier) override;
        std::error_code remove(const std::string& name) override;
        std::optional<std::string> code(const std::string& name) override;
        std::vector<network_details> list(const std::string &query) override;
        int total() override;
    private:
        core::sql::pool::data_source &data_source;
    };
}

#endif // __DAEMON_DOMAIN_NETWORKING_SQL_REPOSITORY__