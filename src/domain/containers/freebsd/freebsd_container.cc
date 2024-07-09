#include <domain/containers/freebsd/freebsd_container.h>
#include <domain/containers/freebsd/freebsd_utils.h>
#include <core/utilities/defer.h>
#include <asio/io_context.hpp>
#include <asio/post.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <libutil.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/jail.h>
#include <sys/mount.h>
#include <jail.h>
#include <termios.h>
#include <spdlog/spdlog.h>
#include <range/v3/view/split.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/range/conversion.hpp>

using namespace ranges;
using namespace core::utilities;
namespace domain::containers::freebsd
{
    freebsd_container::freebsd_container(
        asio::io_context &context,
        operation_details details,
        runtime_listener &listener) : container(std::move(details), listener),
                                      context(context),
                                      file_descriptor(-1),
                                      process_identifier(-1),
                                      buffer(WRITE_BUFFER_SIZE),
                                      stream(nullptr),
                                      network("default"),
                                      logger(spdlog::get("jpod"))
    {
        /* will need to do a strip of network_properties to see if the default network
           setup is present and if not, the default placed in
        */
        // auto only_network = [](const std::string &part) -> bool
        // {
        //     return part.find_first_of("network=") != std::string::npos;
        // };
        // auto parts = details.network_properties | views::split(' ') | views::filter(only_network) | to<std::vector<std::string>>();
        auto parts = details.network_properties | views::split(' ') | to<std::vector<std::string>>();
        for (const auto &part : parts)
        {
            if (auto pos = part.find_first_of("network="); pos != std::string::npos)
            {
                network = part.substr(pos + 1);
            }
        }
    }
    void freebsd_container::initialize()
    {
        if (auto error = mount_file_systems(); error)
        {
            listener.container_failed(details.identifier, error);
        }
        else if (auto error = create_jail(); error)
        {
            listener.container_failed(details.identifier, error);
        }
        else if (error = start_process_in_jail(); error)
        {
            listener.container_failed(details.identifier, error);
        }
        else
        {
            listener.container_initialized(details.identifier, network);
        }
    }
    void freebsd_container::start()
    {
        asio::post([this]()
                   { process_wait(process_identifier); });
        asio::post([this]()
                   { stream->async_wait(
                         asio::posix::stream_descriptor::wait_read,
                         [this](const std::error_code &err)
                         {
                             if (!err)
                             {
                                 listener.container_started(details.identifier);
                                 wait_to_read_from_shell();
                             }
                             else
                             {
                                 listener.container_failed(details.identifier, err);
                             }
                         }); });
    }

    void freebsd_container::register_listener(std::weak_ptr<container_listener> operation_listener)
    {
        logger->info("registering monitor for freebsd-container: {}", details.identifier);

        if (auto listener = operation_listener.lock())
        {
            logger->info("lock acquired for monitor");
            operation_listeners.try_emplace(listener->type(), std::move(listener));
            listener->on_operation_initialization();
        }
        else
        {
            logger->warn("monitor lock expired");
        }
    }

    void freebsd_container::update_parameters(const std::map<std::string, std::string> &parameters)
    {
        std::vector<jailparam> _prmz;
        std::error_code error;
        /* clang-format off */
        defer clean_parameters([&_prmz]{  jailparam_free(&_prmz[0], _prmz.size()); });
        /* clang-format on */
        if (int jail_id = jail_getid(details.identifier.c_str()); jail_id <= 0)
        {
            listener.container_failed(details.identifier, std::error_code{errno, std::system_category()});
        }
        else
        {
            add_parameter(_prmz, std::string("jid"), fmt::format("{}", jail_id));
            for (const auto &[key, value] : details.parameters)
            {
                add_parameter(_prmz, key, value);
            }
            if (int jail_id = jailparam_set(&_prmz[0], _prmz.size(), JAIL_UPDATE); jail_id == -1)
            {
                logger->error("failure in creating jail: JAIL ERR: {} SYS-ERR: {}", jail_errmsg, errno);
                listener.container_failed(details.identifier, std::error_code{errno, std::system_category()});
            }
        }
    }

    bool freebsd_container::setup_pipe(int fd)
    {
        if (auto fd_in_dup = ::dup(fd); fd_in_dup > 0 || close_on_exec(fd_in_dup))
        {
            if (stream)
            {
                stream.reset();
            }
            stream = std::make_unique<asio::posix::stream_descriptor>(context, fd_in_dup);
            return true;
        }
        return false;
    }

    std::error_code freebsd_container::create_jail()
    {
        std::vector<jailparam> parameters;
        /* clang-format off */
        defer clean_parameters([&parameters]{  jailparam_free(&parameters[0], parameters.size()); });
        /* clang-format on */
        add_parameter(parameters, std::string("name"), details.identifier);
        add_parameter(parameters, std::string("vnet"), "");
        add_parameter(parameters, std::string("host.hostname"), details.hostname);
        add_parameter(parameters, std::string("path"), details.container_folder.generic_string().c_str());
        for (const auto &[key, value] : details.parameters)
        {
            add_parameter(parameters, key, value);
        }
        if (int jail_id = jailparam_set(&parameters[0], parameters.size(), JAIL_CREATE); jail_id == -1)
        {
            logger->error("failure in creating jail: JAIL ERR: {} SYS-ERR: {}", jail_errmsg, errno);
            return std::error_code{errno, std::system_category()};
        }
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
            return std::error_code(errno, std::system_category());
        }
        else if (pid == 0)
        {
            if (auto result = (has_user(details.username) ? fetch_user_details(details.username) : fetch_user_details("")); !result)
            {
                logger->error("failed to find matching user details for : {}\n{}", details.username, result.error().message());
                listener.container_failed(details.identifier, result.error());
                _exit(-1);
            }
            else if (int jail_id = jail_getid(details.identifier.c_str()); jail_id > 0)
            {
                if (jail_attach(jail_id) == -1 || chdir(details.workdir.c_str()) == -1)
                {
                    listener.container_failed(details.identifier, std::error_code(errno, std::system_category()));
                    _exit(-errno);
                }
                else
                {
                    context.notify_fork(asio::io_context::fork_child);
                    if (!setup_environment(result.value()))
                    {
                        logger->warn("was not able to set up secure mode");
                    }
                    for (const auto &[key, value] : details.env_vars)
                    {
                        setenv(key.c_str(), value.c_str(), 1);
                    }
                    if (details.env_vars.find("SHELL") == details.env_vars.end())
                    {
                        setenv("SHELL", "/bin/sh", 1);
                    }
                    if (details.env_vars.find("TERM") == details.env_vars.end())
                    {
                        setenv("TERM", "xterm-256color", 1);
                    }
                    std::vector<char *> args;
                    for (const auto &entry : details.command)
                    {
                        args.push_back(const_cast<char *>(entry.c_str()));
                    }
                    args.push_back(nullptr);
                    if (auto err = execvp(args[0], args.data()); err < 0)
                    {
                        perror("execlp failed");
                        listener.container_failed(details.identifier, std::error_code(errno, std::system_category()));
                        _exit(-errno);
                    }
                    return {};
                }
            }
            else
            {
                listener.container_failed(details.identifier, std::error_code(errno, std::system_category()));
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
            logger->info("invoked jailed process");
            return {};
        }
        clean();
        return std::error_code(errno, std::system_category());
    }

    void freebsd_container::read_from_shell()
    {
        stream->async_read_some(
            asio::buffer(buffer),
            [this](std::error_code error, std::size_t bytes_transferred)
            {
                if (!error)
                {
                    if (bytes_transferred > 0)
                    {
                        this->on_operation_output(buffer);
                    }
                    this->wait_to_read_from_shell();
                }
                else
                {
                    if (error != asio::error::eof)
                    {
                        this->on_operation_failure(error);
                    }
                }
            });
    }

    void freebsd_container::wait_to_read_from_shell()
    {

        stream->async_wait(
            asio::posix::stream_descriptor::wait_read,
            [this](std::error_code error)
            {
                if (!error)
                {
                    read_from_shell();
                }
                else if (error != asio::error::eof)
                {
                    on_operation_failure(error);
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

    void freebsd_container::on_operation_failure(std::error_code &error)
    {
        if (auto pos = operation_listeners.find(listener_category::observer); pos != operation_listeners.end())
        {
            if (auto operation_listener = pos->second.lock())
            {
                operation_listener->on_operation_failure(error);
            }
            else
            {
                operation_listeners.erase(pos);
            }
        }
        else if (auto pos = operation_listeners.find(listener_category::runtime); pos != operation_listeners.end())
        {
            if (auto operation_listener = pos->second.lock())
            {
                operation_listener->on_operation_failure(error);
            }
            else
            {
                operation_listeners.erase(pos);
            }
        }
    }
    void freebsd_container::on_operation_output(const std::vector<uint8_t> &content)
    {
        if (auto pos = operation_listeners.find(listener_category::observer); pos != operation_listeners.end())
        {

            if (auto operation_listener = pos->second.lock())
            {
                operation_listener->on_operation_output(content);
            }
            else
            {
                operation_listeners.erase(pos);
            }
        }
        else if (auto pos = operation_listeners.find(listener_category::runtime); pos != operation_listeners.end())
        {

            if (auto operation_listener = pos->second.lock())
            {
                operation_listener->on_operation_output(content);
            }
            else
            {
                operation_listeners.erase(pos);
            }
        }
    }

    std::error_code freebsd_container::mount_file_systems()
    {
        std::error_code error{};
        for (const auto &entry : details.mount_points)
        {
            std::vector<iovec> mount_order_parts;
            /* clangformat off */
            defer free_mount_orders([&mount_order_parts]()
                                    {
                for (auto &entry : mount_order_parts)
                    {
                        free(entry.iov_base);
                    } });
            /* clangformat on */

            add_mount_point_entry(mount_order_parts, "fstype", entry.filesystem);
            add_mount_point_entry(mount_order_parts, "fspath", entry.folder.generic_string());
            if (entry.source && entry.filesystem == "nullfs")
            {
                if (error = create_directories(entry.folder, details.username, details.group); error)
                {
                    return error;
                }
                add_mount_point_entry(mount_order_parts, "target", entry.source.value().generic_string());
                if (error = create_directories(entry.source.value(), details.username, details.group); error)
                {
                    return error;
                }
            }
            else
            {
                add_mount_point_entry(mount_order_parts, "from", entry.filesystem);
            }
            if (auto position = entry.options.find("rw"); position != std::string::npos)
            {
                add_mount_point_entry(mount_order_parts, "rw", fmt::format("{}", 1));
            }
            // if (auto position = entry.options.find("="); position != std::string::npos)
            // {
            //     if (std::stoi(entry.options.substr(position + 1)) == 1777)
            //     {
            //         auto permissions = fs::perms::all | fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all;
            //         if (fs::permissions(entry.folder, permissions, fs::perm_options::add, error); error)
            //         {
            //             return error;
            //         }
            //     }
            // }

            if (nmount(&mount_order_parts[0], mount_order_parts.size(), mount_point_flags(entry.options) | MNT_IGNORE) == -1)
            {
                std::error_code error{errno, std::system_category()};
                logger->error("mounting failed: {}", error.message());
                return error;
            }
            logger->info("mounted: destination: {}", entry.folder.generic_string());
        }
        return {};
    }
    std::error_code freebsd_container::unmount_file_systems()
    {
        logger->info("un-mounting file systems");
        for (const auto &entry : details.mount_points)
        {
            if (auto err = unmount(entry.folder.generic_string().c_str(), MNT_FORCE); err != 0)
            {
                return std::error_code(err, std::system_category());
            }
            logger->warn("fs-type: {} un-mounted: {}", entry.filesystem, entry.folder.generic_string());
        }
        return {};
    }

    freebsd_container::~freebsd_container()
    {
        if (int jail_id = jail_getid(details.identifier.c_str()); jail_id > 0)
        {
            logger->info("SHUTTING DOWN JAIL ID {} ALIAS {}", jail_id, details.identifier);
            jail_remove(jail_id);
        }
        logger->info("proceeding to unmount file-systems");
        if (auto error = unmount_file_systems(); error)
        {
            on_operation_failure(error);
        }
        if (file_descriptor > 0 && process_identifier > 0)
        {
            logger->info("closing file descriptor ");
            close(file_descriptor);
            logger->info("waiting for the end of process");
            waitpid(process_identifier, nullptr, 0);
        }
        listener.container_stopped(details.identifier, network);
    }
}