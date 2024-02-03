#ifndef __DAEMON_DOMAIN_CONTAINERS_CONTAINER__
#define __DAEMON_DOMAIN_CONTAINERS_CONTAINER__

#include <domain/containers/runtime_listener.h>
#include <domain/containers/container_listener.h>
#include <filesystem>
#include <string>
#include <vector>
#include <map>
#include <memory>

namespace fs = std::filesystem;

namespace domain::containers
{
    struct mount_point_entry
    {
        std::string filesystem;
        fs::path folder;
        std::string options;
        uint64_t flags;
    };
    struct operation_details
    {
        std::string identifier;
        std::string username;
        std::string hostname;
        std::map<std::string, std::string> parameters;
        std::map<std::string, std::string> env_vars;
        std::map<std::string, std::string> port_map;
        std::vector<mount_point_entry> mount_points;
        std::string entry_point;
        fs::path container_folder;
        std::string network_properties;
    };
    
    class container
    {

    public:
        container(operation_details details, runtime_listener &listener) : details(std::move(details)), listener(listener)
        {
        }
        virtual void initialize() = 0;
        virtual ~container() = default;
        virtual void start() = 0;
        virtual void register_listener(std::shared_ptr<container_listener> operation_listener) = 0;

    protected:
        operation_details details;
        runtime_listener &listener;
    };
}

#endif // __DAEMON_DOMAIN_CONTAINERS_CONTAINER__