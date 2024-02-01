#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_INTERNAL_UTILS__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_INTERNAL_UTILS__

#include <login_cap.h>
#include <pwd.h>
#include <paths.h>
#include <system_error>
#include <optional>

namespace domain::containers::freebsd
{
    struct user_details
    {
        login_cap_t *lcap;
        passwd *pwd;
    };

    inline auto fetch_user_details(const std::string &username, std::error_code &error) -> std::optional<user_details>
    {
        user_details details{nullptr, nullptr};
        errno = 0;
        if (!username.empty())
        {
            if (details.pwd = getpwnam(username.c_str()); details.pwd == nullptr)
            {
                if (errno != 0)
                {
                    error = std::error_code(errno, std::system_category());
                }
                return std::nullopt;
            }
        }
        else
        {
            uid_t uid = getuid();
            if (details.pwd = getpwuid(uid); details.pwd == nullptr)
            {
                if (errno != 0)
                {
                    error = std::error_code(errno, std::system_category());
                }
                return std::nullopt;
            }
        }
        if (details.lcap = login_getpwclass(details.pwd); details.lcap == nullptr)
        {
            return std::nullopt;
        }
        if (initgroups(details.pwd->pw_name, details.pwd->pw_gid) < 0)
        {
            error = std::error_code(errno, std::system_category());
            return std::nullopt;
        }
        return std::make_optional(details);
    }

    inline auto setup_environment(const user_details &details) -> bool
    {
        if (setgid(details.pwd->pw_gid) != 0)
        {
            return false;
        }
        if (setusercontext(details.lcap, details.pwd, details.pwd->pw_uid, LOGIN_SETALL & ~LOGIN_SETGROUP & ~LOGIN_SETLOGIN) != 0)
        {
            return false;
        }
        login_close(details.lcap);
        setenv("USER", details.pwd->pw_name, 1);
        setenv("HOME", details.pwd->pw_dir, 1);
        setenv("SHELL", *details.pwd->pw_shell ? details.pwd->pw_shell : _PATH_BSHELL, 1);
        if (chdir(details.pwd->pw_dir) < 0)
        {
            return false;
        }
        endpwent();
        return true;
    }
}

#endif // __DAEMON_DOMAIN_CONTAINERS_FREEBSD_INTERNAL_UTILS__