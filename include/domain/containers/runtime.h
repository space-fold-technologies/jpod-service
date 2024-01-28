#ifndef __DAEMON_DOMAIN_CONTAINERS_RUNTIME__
#define __DAEMON_DOMAIN_CONTAINERS_RUNTIME__

#include <memory>
#include <string>
#include <map>

namespace spdlog
{
    class logger;
}

namespace domain::containers
{
    class container;
    class runtime
    {
    public:
        runtime();
        virtual ~runtime();
        void add_container(std::shared_ptr<container> container_ptr);
        void remove_container(std::string& identifier);

    private:
        std::map<std::string, std::shared_ptr<container>> containers;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_RUNTIME__