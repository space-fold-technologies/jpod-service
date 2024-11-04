#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DIRECTORY_RESOLVER__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DIRECTORY_RESOLVER__
#include <filesystem>
#include <system_error>
#include <functional>
#include <domain/images/payload.h>

namespace fs = std::filesystem;
namespace domain::images::instructions
{
    class directory_resolver
    {
    public:
        virtual ~directory_resolver() = default;
        virtual fs::path stage_path(const std::string &label, std::error_code &error) = 0;
        virtual fs::path destination_path(const std::string &identifier, std::error_code &error) = 0;
        virtual fs::path generate_image_path(const std::string &identifier, std::error_code &error) = 0;
        virtual std::error_code extract_image(const std::string &identifier, const std::string &image_identifier) = 0;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DIRECTORY_RESOLVER__
