#ifndef __JPOD_SERVICE_CORE_NETWORKS_INTERFACES_NETWORK_ERRORS__
#define __JPOD_SERVICE_CORE_NETWORKS_INTERFACES_NETWORK_ERRORS__
#include <map>
#include <string>
#include <system_error>

namespace core::networks::interfaces
{

    inline std::map<int, std::string> error_codes = {
        {EACCES, "We don't have permission to do this"},
        {EADDRNOTAVAIL, "Address not available for interface"},
        {EAFNOSUPPORT, "Operation is not supported on sockets"},
        {EBUSY, "Resource is busy"},
        {EEXIST, "An entry already exists"},
        {EFAULT, "Argument references an inaccessible memory area"},
        {EIO, "I/O error"},
        {ENETUNREACH, "Gateway unreachable"},
        {ENOBUFS, "Routing table overflow"},
        {ENOMEM, "Not enough memory available"},
        {ENOTCONN, "Socket is not connected"},
        {ENXIO, "Device does not exist"},
        {ESRCH, "No such process"},
        {EBADF, "FD is not a valid destriptor"},
        {ENOTTY, "FD Associated with another Charactor devce"},
        {EINVAL, "The request or argp argument is not valid"},
        {EFAULT, "The argp argument points the process's allocated address space"}};

    inline auto error_value(int code) -> std::string const
    {
        static const std::string default_error = "net failure of unknown origin occurred";
        if (auto pos = error_codes.find(code); pos != error_codes.end())
        {
            return pos->second;
        }
        return default_error;
    }

    class NetworkErrorCategory : public std::error_category
    {
    public:
        virtual const char *name() const noexcept override
        {
            return "network-error::category";
        }

        virtual std::string message(int code) const override
        {
            return error_value(code);
        };
    };

    const std::error_category &category();

    const std::error_category &category()
    {
        // The category singleton
        static NetworkErrorCategory instance;
        return instance;
    }

    inline std::error_code make_error_code(int code)
    {
        return std::error_code(code, category());
    }
}
#endif // __JPOD_SERVICE_CORE_NETWORKS_INTERFACES_NETWORK_ERRORS__