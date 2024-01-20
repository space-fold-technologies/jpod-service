#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DETAILS__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DETAILS__
#include <string>
#include <filesystem>
namespace domain::images::instructions
{
    struct image_registry_query
    {
        std::string registry;
        std::string name;
        std::string tag;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_DETAILS__