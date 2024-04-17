#ifndef __DAEMON_DOMAIN_NETWORKING_NETWORK_SERVICE__
#define __DAEMON_DOMAIN_NETWORKING_NETWORK_SERVICE__

#include <memory>
#include <system_error>
#include <map>

namespace domain::networking
{
    class address_provider;
    class network_handler;
    class network_repository;
    struct network_entry;
    struct network_details;
};

using network_handler_provider = std::function<std::unique_ptr<domain::networking::network_handler>()>;

namespace domain::networking
{
    class network_service
    {
    public:
        network_service(std::shared_ptr<network_repository> repository, network_handler_provider provider);
        virtual ~network_service();
        std::error_code add(network_entry &entry);
        std::error_code remove(const std::string &name);
        std::error_code join(const std::string &name, const std::string &container_identifier);
        std::error_code leave(const std::string &name, const std::string &container_identifier);
        std::vector<network_details> list(const std::string& query);
        uint32_t total_networks();

    private:
        std::shared_ptr<network_repository> repository;
        network_handler_provider provider;
        std::map<std::string, std::shared_ptr<address_provider>> networks;
    };
}
#endif // __DAEMON_DOMAIN_NETWORKING_NETWORK_SERVICE__