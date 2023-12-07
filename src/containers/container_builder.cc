#include <containers/container.h>
#include <containers/container_builder.h>
#include <core/filesystems/handler.h>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/algorithm/count_if.hpp>
#include <filesystem>
#include <sole.hpp>
#include <fmt/format.h>
#include <sys/mount.h>
#include <spdlog/spdlog.h>
namespace containers
{
    ContainerBuilder::ContainerBuilder(
        asio::io_context &context,
        std::shared_ptr<FileSystemHandler> filesystem_handler) : context(context),
                                                                 logger(spdlog::get("jpod"))
    {
    }
    void ContainerBuilder::build(const Properties &properties)
    {
        id = sole::uuid4().str();
        filesystem_identifier = fmt::format("pod-{}", id);
        if (auto result = filesystem_handler->extract_snapshot(
                properties.snapshot_path,
                filesystem_identifier,
                VolumeType::CONTAINER);
            !result.has_value())
        {
        }
        else if (auto error = mount_filesystems(fs::path(result->absolute_path), properties.mount_points); !error)
        {
            // start first step of the recursion to run all operations including instructions
            fs::path parent_folder = fs::path(result->absolute_path);
            current_directory = parent_folder.generic_string();
        }
        {
            // report error and abort
        };
    }

    ContainerBuilder::~ContainerBuilder()
    {
    }
}