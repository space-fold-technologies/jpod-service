#ifndef __DAEMON_CORE_CONFIGURATION__
#define __DAEMON_CORE_CONFIGURATION__

#include <yaml-cpp/yaml.h>

#include <system_error>
#include <filesystem>
#include <optional>
#include <string>

namespace core::configurations
{
    struct setting_properties
    {
        std::string bridge;
        std::string ip_v4_cidr;
        std::string ip_v6_cidr;
        std::string images_folder;
        std::string containers_folder;
        std::string database_path;
        std::string domain_socket;
        int pool_size;
    };

    inline auto read_from_path(std::string path, std::error_code &error) -> std::optional<setting_properties>
    {
        std::filesystem::path configuration_path(path);
        if (!std::filesystem::exists(configuration_path, error))
        {
            return std::nullopt;
        }
        auto config = YAML::LoadFile(configuration_path.generic_string());
        return setting_properties{
            config["networking"]["bridge"].as<std::string>(),
            config["networking"]["ip-v4-cidr"].as<std::string>(),
            config["networking"]["ip-v6-cidr"].as<std::string>(),
            config["images"]["path"].as<std::string>(),
            config["containers"]["path"].as<std::string>(),
            config["database"]["path"].as<std::string>(),
            config["domain-socket"].as<std::string>(),
            config["database"]["pool-size"].as<int>()};
    }

    inline auto read_from_array(const std::string &content) -> std::optional<setting_properties>
    {
        auto config = YAML::Load(content);
        return setting_properties{
            config["networking"]["bridge"].as<std::string>(),
            config["networking"]["ip-v4-cidr"].as<std::string>(),
            config["networking"]["ip-v6-cidr"].as<std::string>(),
            config["images"]["path"].as<std::string>(),
            config["containers"]["path"].as<std::string>(),
            config["database"]["path"].as<std::string>(),
            config["domain-socket"].as<std::string>(),
            config["database"]["pool-size"].as<int>()};
    }
}

#endif //__DAEMON_CORE_CONFIGURATION__