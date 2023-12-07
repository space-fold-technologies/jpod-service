#ifndef __JPOD_SERVICE_CONTAINERS_MANAGER__
#define __JPOD_SERVICE_CONTAINERS_MANAGER__

#include <tl/expected.hpp>
#include <system_error>
#include <functional>
#include <memory>
#include <map>


namespace spdlog
{
    class logger;
}

namespace core::filesystems
{
    class FileSystemHandler;
}

using namespace core::filesystems;
namespace containers
{
    class Container;
    class ContainerHandler
    {

    public:
        ContainerHandler();
        bool exists(const std::string &id);
        tl::expected<ContainerFS, std::error_code> create(
            const Properties &properties,
            std::shared_ptr<ShellOperator> shell_operator,
            FileSystemHandler &filesystem_handler);

    private:
        std::shared_ptr<spdlog::logger> logger;
        std::map<std::string, std::shared_ptr<Container>> containers;
    };
}
#endif // __JPOD_SERVICE_CONTAINERS_MANAGER__