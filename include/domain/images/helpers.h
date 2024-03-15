#ifndef __DAEMON_DOMAIN_IMAGES_HELPERS__
#define __DAEMON_DOMAIN_IMAGES_HELPERS__

#include <string>
#include <optional>

namespace domain::images::instructions
{
    struct image_registry_query
    {
        std::string registry;
        std::string name;
        std::string tag;
    };

    inline auto resolve_tagged_image_details(const std::string &tagged_image) -> std::optional<image_registry_query>
    {
        if (tagged_image.empty())
        {
            return std::nullopt;
        }
        image_registry_query query{};
        std::string tagged_name = "";
        if (tagged_image.find_last_of("/") == std::string::npos)
        {
            query.registry = "localhost";
            tagged_name = tagged_image;
        }
        else
        {
            auto position = tagged_image.find_last_of("/");
            query.registry = tagged_image.substr(0, position);
            tagged_name = tagged_image.substr(position + 1);
        }

        if (auto position = tagged_name.find_last_of(":"); position != std::string::npos)
        {
            query.name = tagged_name.substr(0, position);
            query.tag = tagged_name.substr(position + 1);
        }
        else
        {
            query.name = tagged_name;
            query.tag = "latest";
        }
        return std::optional{query};
    }
}
#endif // __DAEMON_DOMAIN_IMAGES_HELPERS__