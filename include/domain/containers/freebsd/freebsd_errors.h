#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_ERRORS__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_ERRORS__

#include <system_error>
#include <string>
#include <map>

namespace domain::containers::freebsd
{
    enum class freebsd_error
    {
        unknown_user,
        unknown_uid,
        pwclass_failure,
        initgroup_failure,
    };

    inline std::map<freebsd_error, std::string> freebsd_error_map =
        {
            {freebsd_error::unknown_user, "failed to find specified user"},
            {freebsd_error::unknown_uid, "failed to resolve uid for current user"},
            {freebsd_error::pwclass_failure, "failed to resolve login {pwclass}"},
            {freebsd_error::initgroup_failure, "failed to initialized groups for pwd"}};

    struct freebsd_failure_category : public std::error_category
    {
        freebsd_failure_category() {}
        virtual ~freebsd_failure_category() = default;
        freebsd_failure_category(const freebsd_failure_category &) = delete;
        const char *name() const noexcept override
        {
            return "freebsd failures";
        }

        std::string message(int ec) const override
        {
            static const std::string unknown_error_code("unknown freebsd failure");
            if (auto pos = freebsd_error_map.find(static_cast<freebsd_error>(ec)); pos != freebsd_error_map.end())
            {
                return pos->second;
            }
            return unknown_error_code;
        }
    };

    inline const freebsd_failure_category &__freebsd_failure_category()
    {
        static freebsd_failure_category fc;
        return fc;
    }

    inline const std::error_code make_freebsd_failure(freebsd_error ec) noexcept
    {

        return std::error_code{static_cast<int>(ec), __freebsd_failure_category()};
    };
}

#endif //__DAEMON_DOMAIN_CONTAINERS_FREEBSD_ERRORS__