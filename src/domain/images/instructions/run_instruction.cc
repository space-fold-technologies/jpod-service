#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/run_instruction.h>
#include <core/utilities/freebsd/helper.h>
#include <asio/io_context.hpp>
#include <asio/read.hpp>
#include <asio/post.hpp>
#include <spdlog/spdlog.h>

#if defined(__FreeBSD__) || defined(BSD) && !defined(__APPLE__)
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/mount.h>
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
        std::error_code error{};
        std::vector<mount_point> mount_points;
        int flags = 0;
        flags |= MNT_EMPTYDIR;
        flags &= ~(-(-MNT_RDONLY));
        mount_points.push_back(mount_point{"devfs", "devfs", "dev", flags});
        if (auto entries = resolve_mountpoint_folders(mount_points, error); error)
        {
            listener.on_instruction_complete(identifier, error);
        }
        else if (mount_filesystems(entries, error); error)
        {
            listener.on_instruction_complete(identifier, error);
        } else if(auto error = initialize(); error)
        {
            listener.on_instruction_complete(this->identifier, error);
        } else {
            asio::post([this]()
                   { 
                    in->async_wait(
                         asio::posix::stream_descriptor::wait_read,
                         [this](const std::error_code &err)
                         {
                            read_from_shell();
                         }); 
                    });
        }
    }
    std::error_code run_instruction::initialize()
    {
        disable_stdio_inheritance();
        winsize size = {24, 80, 0, 0};
        context.notify_fork(asio::io_context::fork_prepare);
        int fd;
        auto pid = forkpty(&fd, NULL, NULL, &size);
        if (pid < 0)
        {
            return std::error_code(errno, std::system_category());
        }
        else if (pid == 0)
        {
            //setsid();
            context.notify_fork(asio::io_context::fork_child);
            if (auto result = core::utilities::freebsd::fetch_user_details("root"); !result)
            {
                logger->error("insecure mode in effect error: {}", result.error().message());
                _exit(errno);
            } 
            else if (!core::utilities::freebsd::setup_environment(result.value()))
            {
                        logger->error("was not able to set up secure mode");
                        _exit(errno);
            }
            else if (chdir(current_directory.generic_string().c_str()) == -1 || chroot(".") == -1)
            {
                perror("execlp failed");
                _exit(-errno);
            }
            setenv("TERM", "xterm-256color", 1);
            setenv("SHELL", "/bin/sh", 1);
            auto *target_shell = getenv("SHELL");
            if (target_shell == NULL)
            {
#if defined(__FreeBSD__)
                target_shell = _PATH_BSHELL; // need to find a better way to manage the default shell
#endif
            }
            // std::string argument = fmt::format("\"{}\"", order);
            if (auto err = execlp(target_shell, target_shell, "-c", order.c_str(), NULL); err < 0)
            {
                _exit(errno);
                listener.on_instruction_complete(this->identifier, std::error_code(errno, std::system_category()));
            }
            else
            {
                _exit(0);
                listener.on_instruction_complete(this->identifier, {});
            }
        }

        // set the file descriptor non blocking
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
                    this->listener.on_instruction_data_received(this->identifier, std::vector<uint8_t>(buffer.begin(), buffer.begin() + bytes_transferred));
                    this->in->async_wait(
                        asio::posix::stream_descriptor::wait_read,
                        [this](const std::error_code &err)
                        {
                            if(!err)
                            {
                                read_from_shell();
                            } else 
                            {
                                listener.on_instruction_complete(identifier, err);
                            }
                            
                        });
                }
                else
                {
                    if (err != asio::error::eof)
                    {
                        listener.on_instruction_complete(identifier, err);
                    } else {
                        listener.on_instruction_complete(identifier, {});
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
    std::vector<mount_point_entry> run_instruction::resolve_mountpoint_folders(const std::vector<mount_point> &entries, std::error_code &error)
    {
        std::vector<mount_point_entry> mount_points;
        for (const auto &entry : entries)
        {
                auto folder_path = current_directory / fs::path(entry.destination);
                if (!fs::exists(folder_path, error))
                {
                    if (error)
                    {
                        break;
                    }
                    else if (!fs::create_directories(folder_path, error))
                    {
                        if (error)
                        {
                            break;
                        }
                    }
                }
                mount_points.push_back(mount_point_entry{entry.type, entry.source, folder_path, entry.flags});
        }

        return mount_points;
    }
    #if defined(__FreeBSD__) || defined(BSD) && !defined(__APPLE__)
    bool run_instruction::mount_filesystems(const std::vector<mount_point_entry> &entries, std::error_code &error)
    {
        for (const auto &entry : entries)
        {
            std::vector<iovec> mount_order_parts;
            add_mount_point_entry(mount_order_parts, "fstype", entry.type);
            add_mount_point_entry(mount_order_parts, "fspath", entry.destination.generic_string());
            if (entry.type == "nullfs") 
            {
                add_mount_point_entry(mount_order_parts, "target", entry.source);
            }
            if (!error)
            {
                if (nmount(&mount_order_parts[0], mount_order_parts.size(), entry.flags | MNT_IGNORE) == -1)
                {
                    logger->error("mounting failed: {}", errno);
                    error = std::error_code(errno, std::system_category());
                } else {
                    logger->info("mounted fspath: {}", entry.destination.generic_string());
                }
            }
            for (auto &entry : mount_order_parts)
            {
                free(entry.iov_base);
            }
            if (error)
            {
                break;
            }
        }
        return !error;
    }

    void run_instruction::add_mount_point_entry(std::vector<iovec> &entries, const std::string &key, const std::string &value)
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

#elif defined(__sun__) && defined(__SVR4)
    // put Solaris / illumos specific mount point operations here
    bool run_instruction::mount_filesystems(const std::vector<mount_point_entry> &entries, std::error_code &error)
    {
        std::error_code error;
        return !error;
    }
#else
    bool run_instruction::mount_filesystems(const std::vector<mount_point_entry> &entries, std::error_code &error)
    {
        logger->info("this is a dummy method <this is not the target operating system>");
        for (const auto &entry : entries)
        {
            logger->info("unsupported os mount attempt: {}", entry.folder.generic_string());
        }
        return true;
    }
#endif
std::error_code run_instruction::unmount_filesystems(const std::vector<mount_point> &mount_points, fs::path &directory)
    {
        for (const auto &mount_point : mount_points)
        {
            fs::path folder_path = directory / fs::path(mount_point.destination);
            logger->warn("unmounting: {}", folder_path.generic_string());
#if defined(__FreeBSD__) || defined(BSD) && !defined(__APPLE__)
            if (auto err = unmount(folder_path.generic_string().c_str(), mount_point.flags); err != 0)
            {
                return std::error_code(err, std::system_category());
            }
#elif defined(__sun__) && defined(__SVR4)
            if (auto err = umount2(folder_path.generic_string().c_str(), mount_point.flags); err != 0)
            {
                return std::error_code(err, std::system_category());
            }
#else
            logger->info("not the target operating system");
#endif
        }
        return {};
    }
    void run_instruction::clean()
    {
        if (file_descriptor > 0 && process_identifier > 0)
        {
            close(file_descriptor);
            waitpid(process_identifier, nullptr, 0);
        }
    }
    run_instruction::~run_instruction()
    {
        std::vector<mount_point> mount_points;
        int flags = 0;
        flags |= MNT_EMPTYDIR;
        flags &= ~(-(-MNT_RDONLY));
        mount_points.push_back(mount_point{"devfs", "devfs", "dev", flags});
        if (auto error = unmount_filesystems(mount_points, current_directory); error)
        {
                logger->error("failed to unmount file system :{}", error.message());
        }
        else
        {
                logger->info("finished unmount ops");
        }
    }
}
