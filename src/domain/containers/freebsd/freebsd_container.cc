#include <domain/containers/freebsd/freebsd_container.h>
#include <domain/containers/freebsd/freebsd_utils.h>
#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <array>
#include <libutil.h>
#include <login_cap.h>
#include <poll.h>
#include <pwd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/jail.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/uio.h>
#include <jail.h>
#include <termios.h>
#include <thread>
#include <unistd.h>
#include <filesystem>
#include <range/v3/algorithm/count_if.hpp>
#include <spdlog/spdlog.h>

namespace fs = std::filesystem;

namespace domain::containers::freebsd
{
    freebsd_container::freebsd_container(
        asio::io_context &context,
        container_details details,
        runtime_listener &listener) : container(std::move(details), listener),
                                      context(context),
                                      file_descriptor(-1),
                                      process_identifier(-1),
                                      buffer(WRITE_BUFFER_SIZE),
                                      stream(nullptr),
                                      logger(spdlog::get("jpod"))
    {
    }
    void freebsd_container::initialize()
    {
        if (auto error = mount_file_systems(); error)
        {
            listener.container_failed(details.identifier, std::move(error));
        }
        else if (error = create_jail(); error)
        {
            listener.container_failed(details.identifier, std::move(error));
        }
        else if (error = start_process_in_jail(); error)
        {
            listener.container_failed(details.identifier, std::move(error));
        }
        else
        {
            listener.container_initialized(details.identifier);
        }
    }
    void freebsd_container::start()
    {
        asio::post([this]()
                   { this->process_wait(); });
        asio::post([this]()
                   { this->stream->async_wait(
                         asio::posix::stream_descriptor::wait_read,
                         [this](const std::error_code &err)
                         {
                             if (!err)
                             {
                                 this->read_from_shell();
                             }
                             else
                             {
                                 listener.container_failed(details.identifier, err);
                             }
                         }); });
    }
    void freebsd_container::resize(int columns, int rows)
    {
        struct winsize size;
        size.ws_col = (unsigned short)columns;
        size.ws_row = (unsigned short)rows;
        size.ws_xpixel = 0;
        size.ws_ypixel = 0;
        if (ioctl(file_descriptor, TIOCSWINSZ, &size) < 0)
        {
            listener.container_failed(details.identifier, std::error_code(errno, std::system_category()));
        }
    }
    void freebsd_container::register_listener(std::shared_ptr<container_listener> operation_listener)
    {
        if (this->operation_listener)
        {
            this->operation_listener.reset();
            this->operation_listener = operation_listener;
        }
    }

    void freebsd_container::process_wait()
    {
        pid_t pid;
        int stat;
        do
        {
            pid = waitpid(process_identifier, &stat, 0);
        } while (pid != process_identifier && errno == EINTR);
    }
    bool freebsd_container::setup_pipe(int fd)
    {
        if (auto fd_in_dup = ::dup(fd); fd_in_dup > 0 || close_on_exec(fd_in_dup))
        {
            stream = std::make_unique<asio::posix::stream_descriptor>(context, fd_in_dup);
            return true;
        }
        return false;
    }

    bool freebsd_container::close_on_exec(int fd)
    {
        int flags = fcntl(fd, F_GETFD);
        if (flags < 0)
        {
            return false;
        }
        return (flags & FD_CLOEXEC) == 0 ||
               fcntl(fd, F_SETFD, flags | FD_CLOEXEC) != -1;
    }

    void freebsd_container::disable_stdio_inheritance()
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

    std::error_code freebsd_container::create_jail()
    {
        return {};
    }
    std::error_code freebsd_container::start_process_in_jail()
    {
        disable_stdio_inheritance();
        winsize size = {24, 80, 0, 0};
        context.notify_fork(asio::io_context::fork_prepare);
        int fd;
        auto pid = forkpty(&fd, NULL, NULL, &size);
        if (pid < 0)
        {
            std::error_code(errno, std::system_category());
        }
        else if (pid == 0)
        {
            setsid();
            if (int jail_id = jail_getid(details.identifier.c_str()); jail_id > 0)
            {
                if (jail_attach(jail_id) == -1 || chdir("/") == -1)
                {
                    listener.container_failed(details.identifier, std::error_code(errno, std::system_category()));
                    _exit(-errno);
                }
            }
            context.notify_fork(asio::io_context::fork_child);
            // if (details.username.length() > 0)
            // {
            //     auto results = fetch_user_information(details.username);
            //     if (results.has_value())
            //     {
            //         setup_environment(*results);
            //     }
            // }
            setenv("SHELL", "/bin/sh", 1);
            setenv("TERM", "xterm-256color", 1);
            for (const auto &entry : details.env_vars)
            {
                setenv(entry.first.c_str(), entry.second.c_str(), 1);
            }
            auto target_shell = getenv("SHELL");
            if (target_shell == NULL)
            {
                target_shell = _PATH_BSHELL;
            }
            if (auto err = execlp(target_shell, details.entry_point.c_str(), NULL); err < 0)
            {
                perror("execlp failed");
                listener.container_failed(details.identifier, std::error_code(errno, std::system_category()));
                _exit(-errno);
            }
            else
            {
                listener.container_stopped(details.identifier);
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
                return std::error_code(errno, std::system_category());
            }
            if (!close_on_exec(fd))
            {
                clean();
            }
            if (!setup_pipe(fd))
            {
                clean();
                return std::error_code(errno, std::system_category());
            }
            this->file_descriptor = fd;
            this->process_identifier = pid;
            return {};
        }
        clean();
        return std::error_code(errno, std::system_category());
    }

    void freebsd_container::read_from_shell()
    {
        stream->async_read_some(
            asio::buffer(buffer),
            [this](std::error_code err, std::size_t bytes_transferred)
            {
                if (!err)
                {
                    if (bytes_transferred > 0)
                    {
                        this->on_operation_output(buffer);
                    }
                    // this->stream->async_wait(
                    //     asio::posix::stream_descriptor::wait_read,
                    //     [this](std::error_code error)
                    //     {
                    //         if (!error)
                    //         {
                    //             this->read_from_shell();
                    //         }
                    //         else if (err != asio::error::eof)
                    //         {
                    //             this->on_operation_failure(std::move(error));
                    //         }
                    //     });
                }
                else
                {
                    if (err != asio::error::eof)
                    {
                        this->on_operation_failure(std::move(err));
                    }
                }
            });
    }

    void freebsd_container::clean()
    {
        if (file_descriptor > 0 && process_identifier > 0)
        {
            close(file_descriptor);
            waitpid(process_identifier, nullptr, 0);
        }
    }

    void freebsd_container::on_operation_failure(std::error_code error)
    {
        // TODO: check if the listener is initialized
    }
    void freebsd_container::on_operation_output(const std::vector<uint8_t> &content)
    {
        // TODO: check if the listener is initialized
    }

    std::error_code freebsd_container::mount_file_systems()
    {
        return {};
    }
    std::error_code freebsd_container::unmount_file_systems()
    {
        return {};
    }

    freebsd_container::~freebsd_container()
    {
        if (file_descriptor > 0 && process_identifier > 0)
        {
            close(file_descriptor);
            waitpid(process_identifier, nullptr, 0);
        }
        if (int jail_id = jail_getid(details.identifier.c_str()); jail_id > 0)
        {
            logger->info("SHUTTING DOWN JAIL ID {} ALIAS {}", jail_id, details.identifier);
            jail_remove(jail_id);
        }
        // run un-mount operations here as well
    }
}