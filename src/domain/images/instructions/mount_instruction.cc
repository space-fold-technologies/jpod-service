#include <domain/images/instructions/mount_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/directory_resolver.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <filesystem>
#include <sys/uio.h>



namespace fs = std::filesystem;
namespace domain::images::instructions
{

    mount_instruction::mount_instruction(
        const std::string &identifier,
        directory_resolver &resolver,
        instruction_listener &listener) : instruction("MOUNT", listener),
                                          identifier(identifier),
                                          resolver(resolver),
                                          logger(spdlog::get("jpod"))
    {
    }
    void mount_instruction::execute()
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
        else
        {
            listener.on_instruction_initialized(identifier, name);
            if (mount_filesystems(entries, error); error)
            {
                listener.on_instruction_complete(identifier, error);
            }
            else
            {
                logger->info("mounting complete");
                listener.on_instruction_complete(identifier, {});
            }
        }
    }
    std::vector<mount_point_entry> mount_instruction::resolve_mountpoint_folders(const std::vector<mount_point> &entries, std::error_code &error)
    {
        std::vector<mount_point_entry> mount_points;
        if (auto parent_path = resolver.destination_path(identifier, error); error)
        {
            listener.on_instruction_complete(identifier, error);
        }
        else
        {
            for (const auto &entry : entries)
            {
                auto folder_path = parent_path / fs::path(entry.destination);
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
        }

        return mount_points;
    }
#if defined(__FreeBSD__) || defined(BSD) && !defined(__APPLE__)
    bool mount_instruction::mount_filesystems(const std::vector<mount_point_entry> &entries, std::error_code &error)
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

    void mount_instruction::add_mount_point_entry(std::vector<iovec> &entries, const std::string &key, const std::string &value)
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
    bool mount_instruction::mount_filesystems(const std::vector<mount_point_entry> &entries, std::error_code &error)
    {
        std::error_code error;
        return !error;
    }
#else
    bool mount_instruction::mount_filesystems(const std::vector<mount_point_entry> &entries, std::error_code &error)
    {
        logger->info("this is a dummy method <this is not the target operating system>");
        for (const auto &entry : entries)
        {
            logger->info("unsupported os mount attempt: {}", entry.folder.generic_string());
        }
        return true;
    }
#endif

    mount_instruction::~mount_instruction()
    {
    }
}