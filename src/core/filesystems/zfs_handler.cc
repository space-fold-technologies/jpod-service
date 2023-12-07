#include <core/filesystems/zfs_handler.h>
#include <algorithm>
#include <filesystem>
#include <spdlog/spdlog.h>
#include <range/v3/to_container.hpp>
#include <range/v3/view/split.hpp>
#include <range/v3/view/filter.hpp>
#include <subprocess/pipe.h>
#include <subprocess/process_builder.h>
#include <subprocess/shell_utils.h>
#include <subprocess/environ.h>
#include <subprocess/basic_types.h>

using subprocess::CompletedProcess;
using subprocess::PipeOption;
using subprocess::RunBuilder;
namespace fs = std::filesystem;
namespace rv = ranges::view;

namespace core::filesystems::zfs
{
    ZFSHandler::ZFSHandler(const std::string &pool, const std::string &base_folder) : pool(pool),
                                                                                      base_folder(base_folder),
                                                                                      logger(spdlog::get("jpod"))
    {
    }
    tl::expected<VolumeDetails, std::error_code> ZFSHandler::create_volume(const std::string &identifier, VolumeType type)
    {
        // command to create dataset
        // zfs create -p zroot/jails/templates/${target}

        std::string virtual_path = fmt::format("{}/{}/{}", pool, folders[type], identifier);
        // command used to create a volume
        CompletedProcess process = subprocess::run({"zfs",
                                                    "create",
                                                    "-p",
                                                    virtual_path},
                                                   RunBuilder().cerr(PipeOption::pipe).cout(PipeOption::pipe).check(false));
        if (!process.cerr.empty())
        {
            logger->error("zfs create dataset failed with err\n{}", process.cerr);
            return tl::make_unexpected(std::make_error_code(std::errc::io_error));
        }

        return fetch_volume(identifier, type);
    }
    tl::expected<VolumeDetails, std::error_code> ZFSHandler::fetch_volume(const std::string &identifier, VolumeType type)
    {
        std::string virtual_path = fmt::format("{}/{}/{}", pool, folders[type], identifier);
        CompletedProcess process = subprocess::run({"zfs",
                                                    "list",
                                                    "-Hp",
                                                    virtual_path},
                                                   RunBuilder().cerr(PipeOption::pipe).cout(PipeOption::pipe).check(false));
        if (!process.cerr.empty())
        {
            logger->error("zfs create dataset failed with err\n{}", process.cerr);
            return tl::make_unexpected(std::make_error_code(std::errc::io_error));
        }

        auto empty_line = [](auto &&element)
        { return !element.empty(); };
        auto rows = process.cout | rv::split('\n') | rv::filter(empty_line) | ranges::to<std::vector<std::string>>();
        if (rows.size() == 1)
        {
            std::istringstream stream(rows.at(0));
            std::string name, path;
            std::size_t used, available, refer;
            stream >> name >> used >> available >> refer >> path;
            return volume_details(identifier, name, path, used, available, refer, type);
        }
        return tl::make_unexpected(std::make_error_code(std::errc::io_error));
    }
    tl::expected<VolumeDetails, std::error_code> ZFSHandler::extract_snapshot(
        const std::string &snapshot_path,
        const std::string &target_volume,
        VolumeType type)
    {
        // create the target folder using zfs command {target_name}
        // then extract snapshot to the destination path
        std::string virtual_path = fmt::format("{}/{}/{}", pool, folders[type], target_volume);
        std::string command = fmt::format("gzip -dc {} | zfs receive {}", snapshot_path, virtual_path);
        std::system(command.c_str());
        return fetch_volume(target_volume, type);
    }
    ZFSHandler::~ZFSHandler()
    {
    }
}