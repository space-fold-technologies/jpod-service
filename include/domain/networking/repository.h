#ifndef __DAEMON_DOMAIN_NETWORKING_REPOSITORY__
#define __DAEMON_DOMAIN_NETWORKING_REPOSITORY__

#include <system_error>
#include <string>
#include <optional>

namespace domain::networking
{
    struct network_entry;
    struct network_details;
    struct network_membership;
    class network_repository
    {
    public:
        virtual bool is_known(const std::string &name) = 0;
        virtual std::error_code add(const network_entry &entry) = 0;
        virtual std::error_code update_status(const std::string &name, const std::string &status) = 0;
        virtual bool is_a_member(const std::string &network_name, const std::string &container_identifier) = 0;
        virtual std::error_code join(const std::string &code, const std::string& members, const std::string &container_identifier) = 0;
        virtual std::optional<network_membership> members(const std::string &network_name, const std::string &container_identifier) = 0;
        virtual std::error_code leave(const std::string &network_name, const std::string &container_identifier) = 0;
        virtual std::error_code remove(const std::string &name) = 0;
        virtual std::optional<std::string> code(const std::string& name) = 0;
        virtual std::vector<network_details> list(const std::string &query) = 0;
        virtual int total() = 0;
    };
}

#endif // __DAEMON_DOMAIN_NETWORKING_REPOSITORY__