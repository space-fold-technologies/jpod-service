#ifndef __DAEMON_DOMAIN_NETWORKING_NETWORK_HANDLER__
#define __DAEMON_DOMAIN_NETWORKING_NETWORK_HANDLER__

#include <system_error>
#include <functional>
#include <memory>
namespace domain::networking
{
    class network_handler;
};


namespace domain::networking
{
    struct bridge_order;
    struct network_order;
    struct network_properties;
    struct bridge_result;
    using details_callback = std::function<std::error_code(const bridge_result&)>;
    class network_handler
    {

    public:
        virtual ~network_handler() = default;
        virtual bool has_bridge(const std::string &name, std::error_code &error) = 0;
        virtual std::error_code create_bridge(const bridge_order& order) = 0;
        virtual std::error_code remove_bridge(const std::string &name) = 0;
        virtual std::error_code bridge_container(const network_order &order, details_callback callback) = 0;
        virtual std::error_code leave_bridge(const std::string& name, const std::string& first, std::string& second) = 0;
    };
}

#endif // __DAEMON_DOMAIN_NETWORKING_NETWORK_HANDLER__