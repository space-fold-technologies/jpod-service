#include <containers/container_handler.h>
#include <containers/shell_operator.h>
#include <core/filesystems/handler.h>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/count_if.hpp>
#include <filesystem>
#include <sole.hpp>
#include <jail.h>
#include <fmt/format.h>
#include <sys/mount.h>
#include <sys/jail.h>
#include <sys/param.h>
#include <spdlog/spdlog.h>
#include <thread>
namespace fs = std::filesystem;

namespace containers
{
    ContainerHandler::ContainerHandler() : logger(spdlog::get("jpod"))
    {
    }
    tl::expected<ContainerFS, std::error_code> ContainerHandler::create(
        const Properties &properties,
        std::shared_ptr<ShellOperator> shell_operator,
        FileSystemHandler &filesystem_handler)
    {
        std::error_code error;
        std::string id = sole::uuid4().str();
        if (auto result = filesystem_handler.extract_snapshot(
                properties.snapshot_path,
                fmt::format("pod-{}", id),
                VolumeType::CONTAINER);
            !result.has_value())
        {
            return tl::make_unexpected(result.error());
        }
        else
        {
            
        }
    }
}