#include <domain/images/instructions/run_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <asio/io_context.hpp>
#include <asio/read.hpp>
#include <asio/post.hpp>
#include <spdlog/spdlog.h>

#if defined(__FreeBSD__) || defined(BSD) && !defined(__APPLE__)
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <libutil.h>
#include <sys/wait.h>
#include <paths.h>
#elif defined(__sun__) && defined(__SVR4)
// will look for the header locations in sun and illumos
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h> // this will only work if it is `ORACLE` otherwise, you are doomed
#elif defined(__linux__)
#include <pty.h>
#include <sys/wait.h>
#endif
#include <fmt/format.h>

namespace domain::images::instructions
{
    run_instruction::run_instruction(
        const std::string &identifier,
        const std::string &order,
        asio::io_context &context,
        fs::path &current_directory,
        instruction_listener &listener) : instruction("RUN", listener),
                                          identifier(identifier),
                                          order(order),
                                          context(context),
                                          current_directory(current_directory),
                                          file_descriptor(-1),
                                          process_identifier(-1),
                                          buffer(WRITE_BUFFER_SIZE),
                                          in(nullptr),
                                          logger(spdlog::get("jpod"))

    {
    }
    void run_instruction::execute()
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
    void run_instruction::initialize()
    {
        disable_stdio_inheritance();
        winsize size = {24, 80, 0, 0};
        context.notify_fork(asio::io_context::fork_prepare);
        int fd;
        auto pid = forkpty(&fd, NULL, NULL, &size);
        if (pid < 0)
        {
            listener.on_instruction_complete(this->identifier, std::error_code(errno, std::system_category()));
            return;
        }
        else if (pid == 0)
        {
            setsid();
            context.notify_fork(asio::io_context::fork_child);
            if (chdir(current_directory.c_str()) == -1 || chroot(".") == -1)
            {
                listener.on_instruction_complete(this->identifier, std::error_code(errno, std::system_category()));
                return;
            }
            setenv("TERM", "xterm-256color", 1);
            setenv("SHELL", "/bin/sh", 1);
            auto target_shell = getenv("SHELL");
            if (target_shell == NULL)
            {
#if defined(__FreeBSD__)
                target_shell = _PATH_BSHELL; // need to find a better way to manage the default shell
#endif
            }
            std::string argument = fmt::format("\"{}\"", order);
            if (auto err = execlp(target_shell, target_shell, "-c", argument.c_str(), NULL); err < 0)
            {
                listener.on_instruction_complete(this->identifier, std::error_code(errno, std::system_category()));
            }
            else
            {
                listener.on_instruction_complete(this->identifier, {});
            }
        }

        // set the file descriptor non blocking
        if (int flags = fcntl(fd, F_GETFL); flags != -1)
        {
            if (int ret = fcntl(fd, F_SETFD, flags | O_NONBLOCK); ret == -1)
            {
                listener.on_instruction_complete(this->identifier, std::error_code(errno, std::system_category()));
                return;
            }
            if (!close_on_exec(fd))
            {
                listener.on_instruction_complete(this->identifier, std::error_code(errno, std::system_category()));
                return;
            }
            if (!setup_pipe(fd))
            {
                listener.on_instruction_complete(this->identifier, std::error_code(errno, std::system_category()));
                return;
            }
            this->file_descriptor = fd;
            this->process_identifier = pid;
            listener.on_instruction_initialized(this->identifier, this->name);
        }
        listener.on_instruction_complete(this->identifier, std::error_code(errno, std::system_category()));
    }
    void run_instruction::process_wait()
    {
        pid_t pid;
        int stat;
        do
        {
            pid = waitpid(process_identifier, &stat, 0);
        } while (pid != process_identifier && errno == EINTR);
    }

    bool run_instruction::setup_pipe(int fd)
    {
        if (auto fd_in_dup = ::dup(fd); fd_in_dup > 0)
        {
            in = std::make_unique<asio::posix::stream_descriptor>(context, fd_in_dup);
        }
        else
        {
            return false;
        }
        return true;
    }
    void run_instruction::read_from_shell()
    {
        in->async_read_some(
            asio::buffer(buffer),
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    this->listener.on_instruction_data_received(this->identifier, buffer);
                    this->in->async_wait(
                        asio::posix::stream_descriptor::wait_read,
                        [this](const std::error_code &err)
                        {
                            this->read_from_shell();
                        });
                }
                else
                {
                    if (err != asio::error::eof)
                    {
                        this->listener.on_instruction_complete(this->identifier, err);
                    }
                }
            });
    }

    bool run_instruction::close_on_exec(int fd)
    {
        int flags = fcntl(fd, F_GETFD);
        if (flags < 0)
        {
            return false;
        }
        return (flags & FD_CLOEXEC) == 0 || fcntl(file_descriptor, F_SETFD, flags | FD_CLOEXEC) != -1;
    }
    void run_instruction::disable_stdio_inheritance()
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
    run_instruction::~run_instruction()
    {
    }
}
