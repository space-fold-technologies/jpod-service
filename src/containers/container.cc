#include <asio/post.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <array>
#include <libutil.h>
#include <login_cap.h>
#include <paths.h>
#include <poll.h>
#include <pwd.h>
#include <fcntl.h>
#include <containers/details.h>
#include <containers/container.h>
#include <containers/container_listener.h>
#include <containers/data_types.h>
#include <spdlog/spdlog.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/jail.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <jail.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <filesystem>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/count_if.hpp>

namespace fs = std::filesystem;

namespace containers
{

  Container::Container(
      asio::io_context &context,
      const ContainerDetails &details,
      ContainerListener &listener) : context(context),
                                     details(details),
                                     listener(listener),
                                     file_descriptor(-1),
                                     process_identifier(-1),
                                     buffer(WRITE_BUFFER_SIZE),
                                     in(nullptr),
                                     logger(spdlog::get("jpod")) {}
  void Container::initialize()
  {
    auto err = mount_filesystems();
    if (err)
    {
      listener.on_container_error(details.id, err);
    }
    else
    {
      start_jail();
      if (auto err = start_process_in_jail(); err != 0)
      {
        logger->error("failed to fork to jail");
        listener.on_container_error(details.id, std::error_code(err, std::system_category()));
      }
      else
      {
        listener.on_container_initialized(details.id);
      }
    }
  }
  void Container::start()
  {
    asio::post([this]()
               { this->process_wait(); });
    asio::post([this]()
               { this->in->async_wait(
                     asio::posix::stream_descriptor::wait_read,
                     [this](const std::error_code &err)
                     {
                       if (!err)
                       {
                         this->read_from_shell();
                       }
                       else
                       {
                         listener.on_container_error(details.id, err);
                       }
                     }); });
  }
  void Container::start_jail()
  {
    std::vector<jailparam> parameters;
    std::error_code error;
    add_parameter(parameters, std::string("name"), details.id);
    add_parameter(parameters, std::string("host.hostname"), details.host);
    add_parameter(parameters, std::string("path"), details.path);
    ranges::for_each(
        details.parameters,
        [this, &parameters](const std::pair<std::string, std::string> &entry)
        {
          add_parameter(parameters, entry.first, entry.second);
        });

    if (int jail_id = jailparam_set(&parameters[0], parameters.size(), JAIL_CREATE); jail_id == -1)
    {
      logger->error("failure in creating jail: {}", jail_errmsg);
    }
    jailparam_free(&parameters[0], parameters.size());
  }

  int Container::start_process_in_jail()
  {
    disable_stdio_inheritance();
    winsize size = {24, 80, 0, 0};
    context.notify_fork(asio::io_context::fork_prepare);
    int fd;
    auto pid = forkpty(&fd, NULL, NULL, &size);
    if (pid < 0)
    {
      return errno;
    }
    else if (pid == 0)
    {
      setsid();
      if (int jail_id = jail_getid(details.id.c_str()); jail_id > 0)
      {
        if (jail_attach(jail_id) == -1 || chdir("/") == -1)
        {
          listener.on_container_error(details.id, std::error_code(errno, std::system_category()));
          _exit(-errno);
        }
      }
      context.notify_fork(asio::io_context::fork_child);
      if (details.username.length() > 0)
      {
        auto results = fetch_user_information(details.username);
        if (results.has_value())
        {
          setup_environment(*results);
        }
      }
      setenv("SHELL", "/bin/sh", 1);
      setenv("TERM", "xterm-256color", 1);
      ranges::for_each(
          details.env_vars,
          [this](const auto &entry)
          {
            setenv(entry.first.c_str(), entry.second.c_str(), 1);
          });
      auto target_shell = getenv("SHELL");
      if (target_shell == NULL)
      {
        target_shell = _PATH_BSHELL;
      }
      if (auto err = execlp(target_shell, details.entry_point.c_str(), NULL); err < 0)
      {
        perror("execlp failed");
        listener.on_container_error(details.id, std::error_code(errno, std::system_category()));
        _exit(-errno);
      }
      else
      {
        listener.on_container_shutdown(details.id);
      }
    }
    else
    {
      context.notify_fork(asio::io_context::fork_parent);
    }

    if (int flags = fcntl(fd, F_GETFL); flags != -1)
    {
      if (int ret = fcntl(fd, F_SETFD, flags | O_NONBLOCK); ret == -1)
      {
        clean();
        return errno;
      }
      if (!close_on_exec(fd))
      {
        clean();
      }
      if (!setup_pipe(fd))
      {
        clean();
        return errno;
      }
      this->file_descriptor = fd;
      this->process_identifier = pid;
      return 0;
    }
    clean();
    return errno;
  }

  tl::expected<UserDetails, std::error_code>
  Container::fetch_user_information(const std::string &username)
  {
    uid_t uid;
    UserDetails details{};
    details.lcap = NULL;
    details.pwd = NULL;
    errno = 0;
    if (!username.empty())
    {
      details.pwd = getpwnam(username.c_str());
      if (details.pwd == NULL)
      {
        if (errno)
        {
          logger->error("failed to get pwd for username: {}", username);
          return tl::make_unexpected(std::error_code(errno, std::system_category()));
        }
        logger->error("{}: no such user", username);
        return tl::make_unexpected(std::make_error_code(std::errc::no_message));
      }
      return tl::make_unexpected(std::make_error_code(std::errc::no_message));
    }
    else
    {
      uid = getuid();
      details.pwd = getpwuid(uid);
      if (details.pwd == NULL)
      {
        if (errno)
        {
          logger->error("failed to get pwuid: {}", uid);
          return tl::make_unexpected(std::error_code(errno, std::system_category()));
        }

        logger->error("unknown uid: {}", uid);
        return tl::make_unexpected(std::make_error_code(std::errc::no_message));
      }
      return tl::make_unexpected(std::make_error_code(std::errc::no_message));
    }
    details.lcap = login_getpwclass(details.pwd);
    if (details.lcap == NULL)
      logger->error("getpwclass: %s", details.pwd->pw_name);
    if (initgroups(details.pwd->pw_name, details.pwd->pw_gid) < 0)
      logger->error("initgroups: %s", details.pwd->pw_name);
    return details;
  }
  void Container::setup_environment(const UserDetails &details)
  {
    logger->info("setting up environment");
    if (setgid(details.pwd->pw_gid) != 0)
    {
      logger->error("setgid failed");
    }
    if (setusercontext(details.lcap, details.pwd, details.pwd->pw_uid, LOGIN_SETALL & ~LOGIN_SETGROUP & ~LOGIN_SETLOGIN) != 0)
      logger->error("failed to set user context");
    login_close(details.lcap);
    setenv("USER", details.pwd->pw_name, 1);
    setenv("HOME", details.pwd->pw_dir, 1);
    setenv("SHELL", *details.pwd->pw_shell ? details.pwd->pw_shell : _PATH_BSHELL, 1);
    if (chdir(details.pwd->pw_dir) < 0)
    {
      logger->info("failed to set current directory");
    }
    endpwent();
  }

  void Container::process_wait()
  {
    pid_t pid;
    int stat;
    do
    {
      pid = waitpid(process_identifier, &stat, 0);
    } while (pid != process_identifier && errno == EINTR);
  }

  bool Container::setup_pipe(int fd)
  {
    if (auto fd_in_dup = ::dup(fd); fd_in_dup > 0 || close_on_exec(fd_in_dup))
    {
      in = std::make_unique<asio::posix::stream_descriptor>(context, fd_in_dup);
      return true;
    }
    return false;
  }

  void Container::resize(int columns, int rows)
  {
    struct winsize size;
    size.ws_col = (unsigned short)columns;
    size.ws_row = (unsigned short)rows;
    size.ws_xpixel = 0;
    size.ws_ypixel = 0;
    if (ioctl(file_descriptor, TIOCSWINSZ, &size) < 0)
    {
      listener.on_container_error(details.id, std::error_code(errno, std::system_category()));
    }
  }

  bool Container::close_on_exec(int fd)
  {
    int flags = fcntl(fd, F_GETFD);
    if (flags < 0)
    {
      return false;
    }
    return (flags & FD_CLOEXEC) == 0 ||
           fcntl(fd, F_SETFD, flags | FD_CLOEXEC) != -1;
  }

  void Container::disable_stdio_inheritance()
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

  void Container::read_from_shell()
  {
    in->async_read_some(
        asio::buffer(buffer),
        [this](const std::error_code &err, std::size_t bytes_transferred)
        {
          if (!err)
          {
            if (bytes_transferred > 0)
            {
              this->listener.on_container_data_received(this->details.id, buffer);
            }
            this->in->async_wait(
                asio::posix::stream_descriptor::wait_read,
                [this](const std::error_code &err)
                {
                  if (!err)
                  {
                    this->read_from_shell();
                  }
                  else if (err != asio::error::eof)
                  {
                    listener.on_container_error(this->details.id, err);
                  }
                });
          }
          else
          {
            if (err != asio::error::eof)
            {
              this->listener.on_container_error(this->details.id, err);
            }
          }
        });
  }

  void Container::stop()
  {
    if (int jail_id = jail_getid(details.id.c_str()); jail_id > 0)
    {
      logger->info("SHUTTING DOWN JAIL ID {} ALIAS {}", jail_id, details.id);
      jail_remove(jail_id);
      listener.on_container_error(this->details.id, std::error_code(errno, std::system_category()));
    }
  }
  std::error_code Container::mount_filesystems()
  {
    std::error_code error;
    ranges::for_each(
        details.mount_points,
        [&error, this](const MountPoint &mount_point)
        {
          fs::path folder_path = fs::path(details.path) / fs::path(mount_point.folder);
          if (!fs::is_directory(folder_path))
          {
            this->logger->info("creating folder: {}", folder_path.c_str());
            if (!fs::create_directories(folder_path, error))
            {
              return;
            }
            else
            {
              logger->info("created folder: {}", folder_path.c_str());
            }
          }
          std::vector<iovec> entries;
          add_mount_point_entry(entries, "fstype", mount_point.filesystem);
          add_mount_point_entry(entries, "fspath", folder_path.generic_string());
          add_mount_point_entry(entries, "from", mount_point.filesystem);
          if (nmount(&entries[0], entries.size(), 0) == -1)
          {
            logger->error("mounting failed: {}", errno);
            error = std::error_code(errno, std::system_category());
          }
          else
          {
            mounted_points.try_emplace(folder_path.generic_string(), mount_point.flags);
          }
          ranges::for_each(
              entries,
              [](iovec &entry)
              {
                free(entry.iov_base);
              });
          if (error)
          {
            return;
          }
        });
    return error;
  }
  void Container::add_parameter(
      std::vector<jailparam> &parameters,
      const std::string &key,
      const std::string &value)
  {
    jailparam parameter;
    if (jailparam_init(&parameter, key.c_str()) != 0)
    {
      logger->error("FAILED TO INIT KEY:{} VALUE:{} ERR:{}", key, value, jail_errmsg);
      return;
    }
    else
    {
      if (!value.empty())
      {
        if (jailparam_import(&parameter, value.c_str()) != 0)
        {
          logger->error("FAILED TO ADD KEY:{} VALUE:{} ERR:{}", key, value, jail_errmsg);
          return;
        }
      }
    }
    parameters.push_back(parameter);
  }

  void Container::add_mount_point_entry(std::vector<iovec> &entries, const std::string &key, const std::string &value)
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

  void Container::clean()
  {
    if (file_descriptor > 0 && process_identifier > 0)
    {
      close(file_descriptor);
      waitpid(process_identifier, nullptr, 0);
    }
  }
  Container::~Container()
  {
    if (file_descriptor > 0 && process_identifier > 0)
    {
      close(file_descriptor);
      waitpid(process_identifier, nullptr, 0);
    }

    ranges::for_each(
        mounted_points,
        [this](const std::pair<std::string, uint64_t> &mount_point)
        {
          if (auto err = unmount(mount_point.first.c_str(), mount_point.second); err != 0)
          {
            std::error_code error(err, std::system_category());
            this->logger->error("UNMOUNT FAILURE: {}", error.message());
          }
        });
  }
}