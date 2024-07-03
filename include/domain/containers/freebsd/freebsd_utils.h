#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_INTERNAL_UTILS__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_INTERNAL_UTILS__

#include <login_cap.h>
#include <pwd.h>
#include <paths.h>
#include <system_error>
#include <optional>
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
        auto home = getenv("HOME");
        if (!home || strcmp(home, "") == 0)
        {
            setenv("HOME", "/", 1);
        }
        setenv("SHELL", *details.pwd->pw_shell ? details.pwd->pw_shell : _PATH_BSHELL, 1);
        if (chdir(details.pwd->pw_dir) < 0)
        {
            return false;
        }
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