#ifndef __DAEMON_DOMAIN_NETWORKING_ERRORS__
#define __DAEMON_DOMAIN_NETWORKING_ERRORS__

#include <system_error>
#include <map>
#include <string>

namespace domain::networking
{
    enum class error_code
    {
        exists,
        unknown_bridge,
        bridge_exists,
        bridge_creation_failed,
        invalid_parameters,
        invalid_subnet,
        invalid_address_format,
        running_attachments
    };
    const inline std::map<error_code, std::string> error_map{
        {error_code::exists, "network with given name already exists"},
        {error_code::unknown_bridge, "unknown bridge specified"},
        {error_code::bridge_exists, "bridge with given name already exists"},
        {error_code::bridge_creation_failed, "bridge creation failed"},
        {error_code::invalid_parameters, "invalid parameters supplied for network"},
        {error_code::invalid_subnet, "invalid subnet specified"},
        {error_code::invalid_address_format, "invalid address format"},
        {error_code::running_attachments, "there are running containers attached to the network"}};

    struct network_failure_category : public std::error_category
    {
        network_failure_category() {}
        virtual ~network_failure_category() = default;
        network_failure_category(const network_failure_category &) = delete;
        const char *name() const noexcept override
        {
            return "network failures";
        }

        std::string message(int ec) const override
        {
            static const std::string unknown_error_code("unknown network failure");
            if (auto pos = error_map.find(static_cast<error_code>(ec)); pos != error_map.end())
            {
                return pos->second;
            }
            return unknown_error_code;
        }
    };

    inline const network_failure_category &__network_failure_category()
    {
        static network_failure_category fc;
        return fc;
    }

    inline const std::error_code make_error_code(error_code ec) noexcept
    {

        return std::error_code{static_cast<int>(ec), __network_failure_category()};
    };
}

#endif // __DAEMON_DOMAIN_NETWORKING_ERRORS__