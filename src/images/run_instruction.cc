#include <images/run_instruction.h>
#include <filesystem>
#include <libutil.h>
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
#include <fmt/format.h>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;
namespace images
{
    RunInstruction::RunInstruction(asio::io_context &context,
                                   const std::string &id,
                                   const std::string &source,
                                   const std::string &path,
                                   InstructionListener &listener) : context(context),
                                                                    id(id),
                                                                    source(source),
                                                                    path(path),
                                                                    listener(listener),
                                                                    file_descriptor(-1),
                                                                    process_identifier(-1),
                                                                    buffer(WRITE_BUFFER_SIZE),
                                                                    in(nullptr),
                                                                    logger(spdlog::get("jpod"))
    {
    }
    std::error_code RunInstruction::parse()
    {
        std::error_code err;
        if (path.empty() || !fs::is_directory(path, err))
        {
            return std::make_error_code(std::errc::not_a_directory);
        }
        else if (!err)
        {
            initialize();
        }

        return err;
    }
    void RunInstruction::execute()
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
    void RunInstruction::initialize()
    {
        disable_stdio_inheritance();
        winsize size = {24, 80, 0, 0};
        context.notify_fork(asio::io_context::fork_prepare);
        int fd;
        auto pid = forkpty(&fd, NULL, NULL, &size);
        if (pid < 0)
        {
            listener.on_instruction_runner_completion(this->id, std::error_code(errno, std::system_category()));
            return;
        }
        else if (pid == 0)
        {
            setsid();
            context.notify_fork(asio::io_context::fork_child);
            if (chdir(path.c_str()) == -1 || chroot(".") == -1)
            {
                listener.on_instruction_runner_completion(this->id, std::error_code(errno, std::system_category()));
                return;
            }
            setenv("TERM", "xterm-256color", 1);
            setenv("SHELL", "/bin/sh", 1);
            auto target_shell = getenv("SHELL");
            if (target_shell == NULL)
            {
                target_shell = _PATH_BSHELL;
            }
            if (auto err = execlp(target_shell, target_shell, "-c", fmt::format("\"{}\"", source), NULL); err < 0)
            {
                listener.on_instruction_runner_completion(this->id, std::error_code(errno, std::system_category()));
            }
            else
            {
                listener.on_instruction_runner_completion(this->id, {});
            }
        }

        // set the file descriptor non blocking
        if (int flags = fcntl(fd, F_GETFL); flags != -1)
        {
            if (int ret = fcntl(fd, F_SETFD, flags | O_NONBLOCK); ret == -1)
            {
                listener.on_instruction_runner_completion(this->id, std::error_code(errno, std::system_category()));
                return;
            }
            if (!close_on_exec(fd))
            {
                listener.on_instruction_runner_completion(this->id, std::error_code(errno, std::system_category()));
                return;
            }
            if (!setup_pipe(fd))
            {
                listener.on_instruction_runner_completion(this->id, std::error_code(errno, std::system_category()));
                return;
            }
            this->file_descriptor = fd;
            this->process_identifier = pid;
            listener.on_instruction_runner_initialized(this->id);
        }
        listener.on_instruction_runner_completion(this->id, std::error_code(errno, std::system_category()));
    }
    void RunInstruction::process_wait()
    {
        pid_t pid;
        int stat;
        do
        {
            pid = waitpid(process_identifier, &stat, 0);
        } while (pid != process_identifier && errno == EINTR);
    }

    bool RunInstruction::setup_pipe(int fd)
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
    void RunInstruction::read_from_shell()
    {
        in->async_read_some(
            asio::buffer(buffer),
            [this](const std::error_code &err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    this->listener.on_instruction_runner_data_received(this->id, buffer);
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
                        this->listener.on_instruction_runner_completion(this->id, err);
                    }
                }
            });
    }

    bool RunInstruction::close_on_exec(int fd)
    {
        int flags = fcntl(fd, F_GETFD);
        if (flags < 0)
        {
            return false;
        }
        return (flags & FD_CLOEXEC) == 0 || fcntl(file_descriptor, F_SETFD, flags | FD_CLOEXEC) != -1;
    }
    void RunInstruction::disable_stdio_inheritance()
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

    RunInstruction::~RunInstruction()
    {
    }
}