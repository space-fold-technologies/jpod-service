#include <asio/post.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <libutil.h>
#include <login_cap.h>
#include <paths.h>
#include <poll.h>
#include <pwd.h>
#include <shell/details.h>
#include <shell/terminal.h>
#include <shell/terminal_listener.h>
#include <spdlog/spdlog.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/jail.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <jail.h>
#include <termios.h>
#include <thread>
#include <unistd.h>

namespace shell
{
  Terminal::Terminal(asio::io_context &context, TerminalListener &listener)
      : context(context), listener(listener), file_descriptor(-1),
        process_identifier(-1), buffer(WRITE_BUFFER_SIZE), in(nullptr),
        out(nullptr), logger(spdlog::get("jpod"))

  {
  }

  int Terminal::initialize(const std::string &id, const std::string &username)
  {
    disable_stdio_inheritance();
    winsize size = {24, 80, 0, 0};
    context.notify_fork(asio::io_context::fork_prepare);
    int fd;
    if (int jail_id = jail_getid(id.c_str()); jail_id > 0)
    {
      if (file_descriptor = jail_attach(jail_id) == -1 || chdir("/") == -1)
      {
        return errno;
      }
    }
    auto pid = forkpty(&fd, NULL, NULL, &size);
    if (pid < 0)
    {
      return errno;
    }
    else if (pid == 0)
    {
      setsid();
      context.notify_fork(asio::io_context::fork_child);
      auto results = fetch_user_information(username);
      if (results.has_value())
      {
        setup_environment(*results);
      }
      if (auto target_shell = getenv("SHELL"); target_shell != NULL)
      {
        if (auto err = execlp(target_shell, target_shell, "-i", NULL); err < 0)
        {
          perror("execlp failed");
          _exit(-errno);
        }
      }
    }

    // set the file descriptor non blocking
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

  tl::expected<UserDetails, std::string>
  Terminal::fetch_user_information(const std::string &username)
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
          logger->error("failed to get pwd for username: {}", username);
        else
          logger->error("{}: no such user", username);
      }
    }
    else
    {
      uid = getuid();
      details.pwd = getpwuid(uid);
      if (details.pwd == NULL)
      {
        if (errno)
          logger->error("failed to get pwuid: {}", uid);
        else
          logger->error("unknown uid: {}", uid);
      }
    }
    details.lcap = login_getpwclass(details.pwd);
    if (details.lcap == NULL)
      logger->error("getpwclass: %s", details.pwd->pw_name);
    if (initgroups(details.pwd->pw_name, details.pwd->pw_gid) < 0)
      logger->error("initgroups: %s", details.pwd->pw_name);
    return details;
  }
  void Terminal::setup_environment(const UserDetails &details)
  {

    setenv("PATH", "/bin:/usr/bin", 1);
    if (auto term = getenv("TERM"); term != NULL)
    {
      setenv("TERM", term, 1);
    }
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

  void Terminal::process_wait()
  {
    pid_t pid;
    int stat;
    do
    {
      pid = waitpid(process_identifier, &stat, 0);
    } while (pid != process_identifier && errno == EINTR);
  }

  bool Terminal::setup_pipe(int fd)
  {
    if (auto fd_in_dup = ::dup(fd); fd_in_dup > 0)
    {
      in = std::make_unique<asio::posix::stream_descriptor>(context, fd_in_dup);
    }
    else
    {
      return false;
    }
    if (auto fd_out_dup = ::dup(fd); fd_out_dup > 0)
    {
      out = std::make_unique<asio::posix::stream_descriptor>(context, fd_out_dup);
    }
    else
    {
      return false;
    }
    return true;
  }
  void Terminal::start()
  {
    asio::post([this]()
               { this->process_wait(); });
    asio::post([this]()
               { this->in->async_wait(
                     asio::posix::stream_descriptor::wait_read,
                     [this](const std::error_code &err)
                     { this->read_from_shell(); }); });
    this->listener.on_terminal_initialized();
  }

  void Terminal::resize(int columns, int rows)
  {
    struct winsize size;
    size.ws_col = (unsigned short)columns;
    size.ws_row = (unsigned short)rows;
    size.ws_xpixel = 0;
    size.ws_ypixel = 0;
    if (ioctl(file_descriptor, TIOCSWINSZ, &size) < 0)
    {
      // strerror(errno)
    }
  }

  void Terminal::write(const std::vector<uint8_t> &data)
  {
    out->async_write_some(
        asio::buffer(std::string(data.begin(), data.end())),
        [this](const std::error_code &err, std::size_t bytes_transferred)
        {
          if (!err)
          {
            this->logger->info("written to terminal");
          }
          else
          {
            listener.on_terminal_error(err);
          }
        });
  }

  bool Terminal::close_on_exec(int fd)
  {
    int flags = fcntl(fd, F_GETFD);
    if (flags < 0)
    {
      return false;
    }
    return (flags & FD_CLOEXEC) == 0 ||
           fcntl(file_descriptor, F_SETFD, flags | FD_CLOEXEC) != -1;
  }

  void Terminal::disable_stdio_inheritance()
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

  void Terminal::read_from_shell()
  {
    in->async_read_some(
        asio::buffer(buffer),
        [this](const std::error_code &err, std::size_t bytes_transferred)
        {
          if (!err)
          {
            this->listener.on_terminal_data_received(buffer);
            this->in->async_wait(
                asio::posix::stream_descriptor::wait_read,
                [this](const std::error_code &err)
                { this->read_from_shell(); });
          }
          else
          {
            this->listener.on_terminal_error(err);
          }
        });
  }

  void Terminal::clean()
  {
    if (file_descriptor > 0 && process_identifier > 0)
    {
      close(file_descriptor);
      waitpid(process_identifier, nullptr, 0);
    }
  }

  Terminal::~Terminal()
  {
    if (file_descriptor > 0 && process_identifier > 0)
    {
      close(file_descriptor);
      waitpid(process_identifier, nullptr, 0);
    }
  }

} // namespace shell