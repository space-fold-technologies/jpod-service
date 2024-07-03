#include <domain/images/instructions/mount_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/errors.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/mappings.h>
#include <domain/images/helpers.h>
#include <domain/images/repository.h>
#include <range/v3/algorithm/for_each.hpp>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <sys/mount.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <filesystem>

namespace fs = std::filesystem;
namespace domain::images::instructions
{
    mount_instruction::mount_instruction(
        const std::string &identifier,
        const std::string &order,
        image_repository &repository,
        directory_resolver &resolver,
        instruction_listener &listener) : instruction("MOUNT", listener),
                                          identifier(identifier),
                                          order(order),
                                          repository(repository),
                                          resolver(resolver),
                                          logger(spdlog::get("jpod"))
    {
    }
    void mount_instruction::execute()
    {
        // std::error_code error;
        // if (auto result = resolve_tagged_image_details(order); !result.has_value())
        // {
        //     listener.on_instruction_complete(identifier, make_error_code(error_code::invalid_order_issued));
        // }
        // else if (auto mount_points = repository.fetch_image_mount_points(result->registry, result->repository, result->tag); mount_points.empty())
        // {
        //     listener.on_instruction_complete(identifier, make_error_code(error_code::no_mount_points_present));
        // }
        // else if (auto entries = resolve_mountpoint_folders(mount_points, error); error)
        // {
        //     listener.on_instruction_complete(identifier, error);
        // }
        // else
        // {
        //     listener.on_instruction_initialized(identifier, name);
        //     if (mount_filesystems(entries, error); error)
        //     {
        //         listener.on_instruction_complete(identifier, error);
        //     }
        //     else
        //     {
        //         listener.on_instruction_complete(identifier, {});
        //     }
        // }
    }
    std::vector<mount_point_entry> mount_instruction::resolve_mountpoint_folders(const std::vector<mount_point> &mount_points, std::error_code &error)
    {
        std::vector<mount_point_entry> entries;
        if (auto parent_path = resolver.destination_path(identifier, error); error)
        {
            listener.on_instruction_complete(identifier, error);
        }
        else
        {
            for (const auto &mount_point : mount_points)
            {
                auto folder_path = parent_path / fs::path(mount_point.folder);
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
                entries.push_back(mount_point_entry{mount_point.filesystem, folder_path, mount_point.options, mount_point.flags});
            }
            return entries;
        }

        return entries;
    }
#if defined(__FreeBSD__) || defined(BSD) && !defined(__APPLE__)
    bool mount_instruction::mount_filesystems(const std::vector<mount_point_entry> &entries, std::error_code &error)
    {
        for (const auto &entry : entries)
        {
            std::vector<iovec> mount_order_parts;
            add_mount_point_entry(mount_order_parts, "fstype", entry.filesystem);
            add_mount_point_entry(mount_order_parts, "fspath", entry.folder.generic_string());
            add_mount_point_entry(mount_order_parts, "from", entry.filesystem);
            if (auto position = entry.options.find("rw"); position != std::string::npos)
            {
                add_mount_point_entry(mount_order_parts, "rw", fmt::format("{}", 1));
            }
            if (auto position = entry.options.find("="); position != std::string::npos)
            {
                if (std::stoi(entry.options.substr(position + 1)) == 1777)
                {
                    auto permissions = fs::perms::all | fs::perms::owner_all | fs::perms::group_all | fs::perms::others_all;
                    if (fs::permissions(entry.folder, permissions, fs::perm_options::add, error); error)
                    {
                        continue;
                    }
                }
            }
            if (!error)
            {
                if (nmount(&mount_order_parts[0], mount_order_parts.size(), entry.flags) == -1)
                {
                    logger->error("mounting failed: {}", errno);
                    error = std::error_code(errno, std::system_category());
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