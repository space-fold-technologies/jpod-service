#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_INTERNAL_UTILS__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_INTERNAL_UTILS__

#include <domain/containers/freebsd/freebsd_errors.h>
#include <tl/expected.hpp>
#include <login_cap.h>
#include <pwd.h>
#include <paths.h>
#include <jail.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <cctype>

namespace domain::containers::freebsd
{
    struct user_details
    {
        login_cap_t *lcap;
        passwd *pwd;
        bool not_root;
    };

    inline auto fetch_user_details(const std::string &username) -> tl::expected<user_details, std::error_code>
    {
        user_details details{};
        details.not_root = username.find_first_of("root") == std::string::npos;
        errno = 0;
        if (details.pwd = username.empty() ? getpwuid(getuid()) : getpwnam(username.c_str()); details.pwd == nullptr)
        {
            if (errno != 0)
            {
                return tl::make_unexpected(std::error_code{errno, std::system_category()});
            }
            return tl::make_unexpected(make_freebsd_failure(freebsd_error::unknown_user));
        }
        else if (details.lcap = login_getpwclass(details.pwd); details.lcap == nullptr)
        {
            if (errno != 0)
            {
                return tl::make_unexpected(std::error_code{errno, std::system_category()});
            }
            return tl::make_unexpected(make_freebsd_failure(freebsd_error::pwclass_failure));
        }
        else if (initgroups(details.pwd->pw_name, details.pwd->pw_gid) < 0)
        {
            if (errno != 0)
            {
                return tl::make_unexpected(std::error_code{errno, std::system_category()});
            }
            return tl::make_unexpected(make_freebsd_failure(freebsd_error::pwclass_failure));
        }
        
        return details;
    }

    inline auto setup_environment(const user_details &details) -> bool
    {
        if (setgid(details.pwd->pw_gid) != 0)
        {
            return false;
        }
        auto flags = details.not_root ? LOGIN_SETALL & ~LOGIN_SETGROUP & ~LOGIN_SETLOGIN
		    : LOGIN_SETPATH | LOGIN_SETENV;
        if (setusercontext(details.lcap, details.pwd, details.pwd->pw_uid, flags) != 0)
        {
            return false;
        }
        login_close(details.lcap);
        setenv("USER", details.pwd->pw_name, 1);
        setenv("HOME", details.pwd->pw_dir, 1);
        setenv("SHELL", *details.pwd->pw_shell ? details.pwd->pw_shell : _PATH_BSHELL, 1);
        endpwent();
        return true;
    }

    inline auto add_parameter(std::vector<jailparam> &parameters, const std::string &key, const std::string &value) -> void
    {
        jailparam parameter;
        if (jailparam_init(&parameter, key.c_str()) != 0)
        {
            spdlog::get("jpod")->error("FAILED TO INIT KEY:{} VALUE:{} ERR:{}", key, value, jail_errmsg); // will need a std::error_code equivalent of jail errors(might have already written it)
            return;
        }
        else
        {
            auto value_copy = std::string(value);
            value_copy.erase(std::remove_if(value_copy.begin(), value_copy.end(), ::isspace), value_copy.end());
            if (!value_copy.empty())
            {

                if (jailparam_import(&parameter, value_copy.c_str()) != 0)
                {
                    spdlog::get("jpod")->error("FAILED TO ADD KEY:{} VALUE:{} ERR:{}", key, value, jail_errmsg);

                    return;
                }
            }
        }
        parameters.push_back(parameter);
    }

    inline auto add_mount_point_entry(std::vector<iovec> &entries, const std::string &key, const std::string &value) -> void
    {
        iovec key_entry{};
        key_entry.iov_base = strdup(key.c_str());
        key_entry.iov_len = key.length() + 1;
        entries.push_back(key_entry);
        iovec value_entry{};
        value_entry.iov_base = strdup(value.c_str());
        value_entry.iov_len = value.length() + 1;
        entries.push_back(value_entry);
    }

    inline auto close_on_exec(int fd) -> bool
    {
        int flags = fcntl(fd, F_GETFD);
        if (flags < 0)
        {
            return false;
        }
        return (flags & FD_CLOEXEC) == 0 ||
               fcntl(fd, F_SETFD, flags | FD_CLOEXEC) != -1;
    }

    inline auto disable_stdio_inheritance() -> void
    {
        auto fd_closer = [](int fd, int set)
        {
            int flags;
            int r;

            flags = 0;
            if (set)
                flags = FD_CLOEXEC;

            do
                r = fcntl(fd, F_SETFD, flags);
            while (r == -1 && errno == EINTR);

            if (r)
                return errno;

            return 0;
        };
        for (int fd = 0; fd < 0; fd++)
        {
            if (fd_closer(fd, 1) && fd > 15)
                break;
        }
    }

    inline auto process_wait(int process_identifier) -> void
    {
        pid_t pid;
        int stat;
        do
        {
            pid = waitpid(process_identifier, &stat, 0);
        } while (pid != process_identifier && errno == EINTR);
    }

}

#endif // __DAEMON_DOMAIN_CONTAINERS_FREEBSD_INTERNAL_UTILS__