#ifndef __DAEMON_DOMAIN_IMAGES_HELPERS__
#define __DAEMON_DOMAIN_IMAGES_HELPERS__

#include <string>
#include <optional>
#include <regex>
#include <algorithm>

namespace domain::images::instructions
{
    const std::regex DOMAIN_PATTERN("^(?!-)[A-Za-z0-9-]+([\\-\\.]{1}[a-z0-9]+)*\\.[A-Za-z]{2,6}$");
    struct image_registry_query
    {
        std::string registry;
        std::string repository;
        std::string tag;
    };

    inline auto has_domain(const std::string &str) -> bool
    {
        if (auto pos = str.find_first_of('/'); pos != std::string::npos)
        {
            auto target = str.substr(0, pos);
            return std::regex_match(target, DOMAIN_PATTERN);
        }
        return false;
    }

    inline auto resolve_tagged_image_details(const std::string &tagged_image) -> std::optional<image_registry_query>
    {
        if (tagged_image.empty())
        {
            return std::nullopt;
        }
        image_registry_query query{};
        std::string tagged_name = "";
        bool is_official = false;
        if (auto count = std::count(tagged_image.begin(), tagged_image.end(), '/'); count == 0 || !has_domain(tagged_image))
        {
            query.registry = "index.docker.io";
            tagged_name = tagged_image;
            is_official = count == 0 && query.registry == "index.docker.io";
        }
        else
        {
            auto position = tagged_image.find_first_of("/");
            query.registry = tagged_image.substr(0, position);
            tagged_name = tagged_image.substr(position + 1);
        }

        if (auto position = tagged_name.find_last_of(":"); position != std::string::npos)
        {
            query.repository = (is_official ? "library/" : "") + tagged_name.substr(0, position);
            query.tag = tagged_name.substr(position + 1);
        }
        else
        {
            query.repository = (is_official ? "library/" : "") + tagged_name;
            query.tag = "latest";
        }
        return std::optional{query};
    }
}
#endif // __DAEMON_DOMAIN_IMAGES_HELPERS__