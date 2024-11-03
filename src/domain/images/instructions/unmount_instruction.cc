#include <domain/images/instructions/unmount_instruction.h>
#include <domain/images/instructions/instruction_listener.h>
#include <domain/images/instructions/directory_resolver.h>
#include <domain/images/instructions/errors.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/uio.h>

namespace domain::images::instructions
{
    unmount_instruction::unmount_instruction(
        const std::string &identifier,
        directory_resolver &resolver,
        instruction_listener &listener) : instruction("UNMOUNT", listener),
                                          identifier(identifier),
                                          resolver(resolver),
                                          logger(spdlog::get("jpod"))
    {
    }
    void unmount_instruction::execute()
    {
        std::error_code error{};
        std::vector<mount_point> mount_points;
        int flags = 0;
        flags |= MNT_EMPTYDIR;
        flags &= ~(-(-MNT_RDONLY));
        mount_points.push_back(mount_point{"devfs", "devfs", "dev", flags});
        if (auto parent_path = resolver.destination_path(identifier, error); error)
        {
            listener.on_instruction_complete(identifier, error);
        }
        else
        {
            listener.on_instruction_initialized(identifier, name);
            if (error = unmount_filesystems(mount_points, parent_path); error)
            {
                listener.on_instruction_complete(identifier, error);
            }
            else
            {
                listener.on_instruction_complete(identifier, {});
                logger->info("finished unmount ops");
            }
        }
    }
    std::error_code unmount_instruction::unmount_filesystems(const std::vector<mount_point> &mount_points, fs::path &directory)
    {
        for (const auto &mount_point : mount_points)
        {
            fs::path folder_path = directory / fs::path(mount_point.destination);

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

    unmount_instruction::~unmount_instruction()
    {
    }
}