#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_INTERNAL_UTILS__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_INTERNAL_UTILS__

#include <domain/containers/freebsd/freebsd_errors.h>
#include <range/v3/view/split.hpp>
#include <range/v3/to_container.hpp>
#include <tl/expected.hpp>
#include <spdlog/spdlog.h>
#include <sys/mount.h>
#include <login_cap.h>
#include <filesystem>
#include <grp.h>
#include <pwd.h>
#include <paths.h>
#include <jail.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

namespace domain::containers::freebsd
{
    struct user_details
    {
        login_cap_t *lcap;
        passwd *pwd;
        bool not_root;
    };

    static std::map<std::string, int, std::less<>> flag_map = {
        {"async", MNT_ASYNC},
        {"atime", -MNT_NOATIME},
        {"exec", -MNT_NOEXEC},
        {"suid", -MNT_NOSUID},
        {"symfollow", -MNT_NOSYMFOLLOW},
        {"rdonly", MNT_RDONLY},
        {"sync", MNT_SYNCHRONOUS},
        {"union", MNT_UNION},
        {"userquota", 0},
        {"groupquota", 0},
        {"clusterr", -MNT_NOCLUSTERR},
        {"clusterw", -MNT_NOCLUSTERW},
        {"suiddir", MNT_SUIDDIR},
        {"snapshot", MNT_SNAPSHOT},
        {"multilabel", MNT_MULTILABEL},
        {"acls", MNT_ACLS},
        {"nfsv4acls", MNT_NFS4ACLS},
        {"automounted", MNT_AUTOMOUNTED},
        {"untrusted", MNT_UNTRUSTED},

        /* Control flags. */
        {"force", MNT_FORCE},
        {"update", MNT_UPDATE},
        {"ro", MNT_RDONLY},
        {"rw", -MNT_RDONLY},
        {"cover", -MNT_NOCOVER},
        {"emptydir", MNT_EMPTYDIR},

        // ignore these
        {"private", 0},
        {"rprivate", 0},
        {"rbind", 0},
        {"nodev", 0},
        {"bind", 0},
    };

    static auto split_option(std::string_view option) -> std::tuple<std::string_view, std::string_view>
    {
        if (auto sep = option.find_first_of("="); sep == std::string::npos)
        {
            return std::make_tuple(option, "");
        }
        else
        {
            return {option.substr(0, sep), option.substr(sep + 1)};
        }
    }

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

    inline auto mount_point_flags(const std::string &options) -> int32_t
    {
        int32_t flags = 0;
        if (options.find(',') != std::string::npos)
        {
            for (const auto &option : options | ranges::views::split(',') | ranges::to<std::vector<std::string>>())
            {
                auto [key, value] = split_option(option);
                if (auto pos = flag_map.find(key); pos != flag_map.end())
                {
                    auto flag = pos->second;
                    if (pos->second > 0)
                    {
                        flags |= flag;
                    }
                    else
                    {
                        flags &= ~(-flag);
                    }
                }
            }
            return flags;
        }

        if (auto pos = flag_map.find(options); pos != flag_map.end())
        {
            auto flag = pos->second;
            if (pos->second > 0)
            {
                flags |= flag;
            }
            else
            {
                flags &= ~(-flag);
            }
        }
        return flags;
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

    inline auto create_directories(const fs::path &path, const std::string &user, const std::string &group) -> std::error_code
    {
        std::error_code error{};
        auto logger = spdlog::get("jpod");
        if (!fs::exists(path) || !fs::is_directory(path))
        {
            auto group_size = sysconf(_SC_NGROUPS_MAX) + 1;
            std::vector<gid_t> groups;
            groups.reserve(group_size);
            auto ngroup = static_cast<int>(group_size);
            if (fs::create_directories(path, error); error)
            {
                return error;
            }
            logger->info("directories created");
            if (auto *pwd = getpwnam(user.c_str()); pwd == nullptr)
            {
                return make_freebsd_failure(freebsd_error::unknown_user);
            }
            else if (auto err = getgrouplist(pwd->pw_name, pwd->pw_gid, &groups[0], &ngroup); err != 0)
            {
                return std::error_code{err, std::system_category()};
            }
            else
            {
                for (const auto &group_id : groups)
                {
                    auto *grp = getgrgid(group_id);
                    logger->info("GROUP FOUND: {}", grp->gr_name);
                    if (std::strcmp(grp->gr_name, group.c_str()) == 0)
                    {
                        if (auto err = chown(path.c_str(), pwd->pw_uid, grp->gr_gid); err < 0)
                        {

                            return std::error_code{err, std::system_category()};
                        }
                        else
                        {
                            logger->info("directory ownership set");
                        }
                    }
                }
                logger->info("directory permissions set");
                return {};
            }
            return make_freebsd_failure(freebsd_error::chown_failure);
        }
        // auto permissions = fs::perms::all | fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all;
        // if (fs::permissions(path, permissions, fs::perm_options::add, error); error)
        // {
        //     return error;
        // }
        mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
        if (auto err = chmod(path.c_str(), mode); err < 0)
        {
            return std::error_code{err, std::system_category()};
        }
        logger->info("set directory mode to correct permissions");
        return {};
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