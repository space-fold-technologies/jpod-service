#include <containers/shell_operator.h>
#include <containers/shell_operator_listener.h>
#if defined(__OpenBSD__) || defined(__APPLE__)
#include <util.h>
#elif defined(__FreeBSD__)
#include <libutil.h>
#else
#include <pty.h>
#endif
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <paths.h>
#include <sys/wait.h>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <asio/post.hpp>
#include <thread>
#include <spdlog/spdlog.h>

namespace containers
{
    ShellOperator::ShellOperator(
        asio::io_context &context,
        ShellOperatorListener &listener) : context(context),
                                           listener(listener),
                                           file_descriptor(-1),
                                           process_identifier(-1),
                                           buffer(WRITE_BUFFER_SIZE),
                                           in(nullptr),
                                           out(nullptr),
                                           logger(spdlog::get("jpod")),
                                           io_closed(false)
    {
    }

    void ShellOperator::start()
    {
        asio::post([this]()
                   { this->process_wait(); });
        asio::post([this]()
                   { this->in->async_wait(
                         asio::posix::stream_descriptor::wait_read,
                         [this](const std::error_code &err)
                         {
                             this->read_from_shell();
                         }); });
    }
    void ShellOperator::stop()
    {
        in->close();
        out->close();
        clean();
    }
    int ShellOperator::initialize(const std::string path)
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
            context.notify_fork(asio::io_context::fork_child);
            if (chdir(path.c_str()) == -1 || chroot(".") == -1)
            {
                return -1;
            }
            setenv("TERM", "xterm-256color", 1);
            setenv("SHELL", "/bin/sh", 1);
            auto target_shell = getenv("SHELL");
            if (target_shell == NULL)
            {
                target_shell = _PATH_BSHELL;
            }
            if (auto err = execlp(target_shell, target_shell, "-i", NULL); err < 0)
            {
                perror("execlp failed");
                _exit(-errno);
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
    void ShellOperator::process_wait()
    {
        pid_t pid;
        int stat;
        do
        {
            pid = waitpid(process_identifier, &stat, 0);
        } while (pid != process_identifier && errno == EINTR);
    }

    bool ShellOperator::setup_pipe(int fd)
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
    void ShellOperator::read_from_shell()
    {
        in->async_read_some(
            asio::buffer(buffer),
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
      if(!err)
      {
        this->listener.on_shell_operator_data_received(buffer);
        this->in->async_wait(
                     asio::posix::stream_descriptor::wait_read,
                     [this](const std::error_code &err)
                     {
                       this->read_from_shell();
                     });
      } else {
         if(err != asio::error::eof)
          {
            this->listener.on_shell_operator_error(err);
         } else {
            this->listener.on_shell_closed();
            this->io_closed = true;
         }
      } });
    }
    void ShellOperator::write(const std::string &data)
    {
        out->async_write_some(asio::buffer(data),
                              [this](const std::error_code &err, std::size_t bytes_transferred)
                              {
                                  if (!err)
                                  {
                                      this->logger->info("written to shell");
                                  }
                                  else
                                  {
                                      listener.on_shell_operator_error(err);
                                  }
                              });
    }
    bool ShellOperator::is_available()
    {
        // implement blocking based wait to avoid writing on failure
        std::error_code error;
        this->in->wait(asio::posix::stream_descriptor::wait_read, error);
        return !error;
    }

    bool ShellOperator::is_closed()
    {
        return io_closed;
    }

    bool ShellOperator::close_on_exec(int fd)
    {
        int flags = fcntl(fd, F_GETFD);
        if (flags < 0)
        {
            return false;
        }
        return (flags & FD_CLOEXEC) == 0 || fcntl(file_descriptor, F_SETFD, flags | FD_CLOEXEC) != -1;
    }
    void ShellOperator::disable_stdio_inheritance()
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
    void ShellOperator::clean()
    {
        if (file_descriptor > 0 && process_identifier > 0)
        {
            close(file_descriptor);
            waitpid(process_identifier, nullptr, 0);
        }
    }
    ShellOperator::~ShellOperator()
    {
        if (file_descriptor > 0 && process_identifier > 0)
        {
            close(file_descriptor);
            waitpid(process_identifier, nullptr, 0);
        }
    }
}