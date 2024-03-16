#include <domain/containers/freebsd/freebsd_terminal.h>
#include <domain/containers/freebsd/freebsd_utils.h>
#include <domain/containers/terminal_listener.h>
#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <libutil.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/jail.h>
#include <spdlog/spdlog.h>

namespace domain::containers::freebsd
{
    freebsd_terminal::freebsd_terminal(asio::io_context &context,
                                       const std::string &identifier,
                                       terminal_listener &listener) : context(context),
                                                                      identifier(identifier),
                                                                      listener(listener),
                                                                      file_descriptor(-1),
                                                                      process_identifier(-1),
                                                                      buffer(WRITE_BUFFER_SIZE),
                                                                      in(nullptr),
                                                                      out(nullptr),
                                                                      logger(spdlog::get("jpod")) {}
    std::error_code freebsd_terminal::initialize()
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
            setsid();
            logger->info("id: {}", identifier);
            if (int jail_id = jail_getid(identifier.c_str()); jail_id > 0)
            {
                if (jail_attach(jail_id) == -1 || chdir("/") == -1)
                {
                    listener.on_terminal_error(std::error_code(errno, std::system_category()));
                    return {};
                }
                else
                {
                    context.notify_fork(asio::io_context::fork_child);
                    std::error_code error;
                    if (auto results = fetch_user_details("", error); error)
                    {
                        logger->error("insecure mode in effect error: {}", error.message());
                    }
                    else if (!setup_environment(*results))
                    {
                        logger->error("was not able to set up secure mode");
                    }
                    setenv("SHELL", "/bin/sh", 1);
                    setenv("TERM", "xterm-256color", 1);
                    if (auto target_shell = getenv("SHELL"); target_shell != NULL)
                    {
                        if (auto err = execlp(target_shell, target_shell, "-i", NULL); err < 0)
                        {
                            listener.on_terminal_error(std::error_code(errno, std::system_category()));
                            return {};
                        }
                    }
                    return {};
                }
            } else {
                listener.on_terminal_error(std::error_code(errno, std::system_category()));
                return {};
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
            return std::error_code{};
        }
        clean();
        return std::error_code(errno, std::system_category());
    }

    void freebsd_terminal::start()
    {

        asio::post([this]()
                   { process_wait(process_identifier); });
        asio::post([this]()
                   { this->in->async_wait(
                         asio::posix::stream_descriptor::wait_read,
                         [this](const std::error_code &error)
                         {
                             if (!error)
                             {
                                 this->listener.on_terminal_initialized();
                                 wait_to_read_from_shell();
                             }
                             else
                             {
                                 listener.on_terminal_error(error);
                             }
                         }); });
    }
    void freebsd_terminal::resize(uint32_t columns, uint32_t rows)
    {
        struct winsize size;
        size.ws_col = (unsigned short)columns;
        size.ws_row = (unsigned short)rows;
        size.ws_xpixel = 0;
        size.ws_ypixel = 0;
        if (ioctl(file_descriptor, TIOCSWINSZ, &size) < 0)
        {
            listener.on_terminal_error(std::error_code(errno, std::system_category()));
        }
    }
    void freebsd_terminal::write(const std::vector<uint8_t> &content)
    {
        out->async_write_some(
            asio::buffer(std::string(content.begin(), content.end())),
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
    bool freebsd_terminal::setup_pipe(int fd)
    {
        if (auto fd_in_dup = ::dup(fd); fd_in_dup <= 0)
        {
            return false;
        }
        else if (auto fd_out_dup = ::dup(fd); fd_out_dup <= 0)
        {
            return false;
        }
        else
        {
            in = std::make_unique<asio::posix::stream_descriptor>(context, fd_in_dup);
            out = std::make_unique<asio::posix::stream_descriptor>(context, fd_out_dup);
        }
        return true;
    }
    void freebsd_terminal::wait_to_read_from_shell()
    {
        this->in->async_wait(
            asio::posix::stream_descriptor::wait_read,
            [this](const std::error_code &error)
            {
                if (!error)
                {
                    this->read_from_shell();
                }
                else if (error != asio::error::eof)
                {
                    this->listener.on_terminal_error(error);
                }
            });
    }
    void freebsd_terminal::read_from_shell()
    {
        in->async_read_some(
            asio::buffer(buffer),
            [this](const std::error_code &error, std::size_t bytes_transferred)
            {
                if (!error)
                {
                    if (bytes_transferred > 0)
                    {
                        this->listener.on_terminal_data_received(buffer);
                        this->in->async_wait(
                            asio::posix::stream_descriptor::wait_read,
                            [this](const std::error_code &err)
                            {
                                this->wait_to_read_from_shell();
                            });
                    }
                }
                else
                {
                    this->listener.on_terminal_error(error);
                }
            });
    }
    void freebsd_terminal::clean()
    {
        if (file_descriptor > 0 && process_identifier > 0)
        {
            close(file_descriptor);
            waitpid(process_identifier, nullptr, 0);
        }
    }
    freebsd_terminal::~freebsd_terminal()
    {
        clean();
        buffer.clear();
        in.reset();
        out.reset();
    }
}