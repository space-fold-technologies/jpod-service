#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_IMPORT_RESOLVER__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_IMPORT_RESOLVER__
#include <filesystem>
#include <system_error>
#include <functional>

namespace fs = std::filesystem;
namespace domain::images::instructions
{
    class import_resolver
    {
    public:
        virtual fs::path archive_file_path() = 0;
        virtual fs::path generate_image_path(const std::string &identifier, std::error_code &error) = 0;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_IMPORT_RESOLVER__
