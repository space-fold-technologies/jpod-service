#ifndef __DAEMON_CORE_OCI_PAYLOADS__
#define __DAEMON_CORE_OCI_PAYLOADS__

#include <string>
#include <vector>
#include <chrono>
#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

namespace core::oci
{
    enum class authorization_variant
    {
        basic_auth,
        registry_token,
        none
    };
    inline auto authorization_variant_from_str(const std::string &type) -> std::optional<authorization_variant>
    {
        if (type == "NONE")
            return authorization_variant::none;
        if(type == "TOKEN")
            return authorization_variant::registry_token;
        if(type == "BASIC_AUTH")
            return authorization_variant::basic_auth;
        return std::nullopt;
    }
    struct registry_credentials
    {
        std::string name;
        authorization_variant variant;
        std::string authorization_server;
        std::string registry;
        std::string credentials;
    };
    struct image_fetch_order
    {
        std::string registry;
        std::string repository;
        std::string tag;
        std::string architecture;
        std::string operating_system;
        fs::path destination;
    };
    struct progress_update
    {
        std::string image_digest;
        std::string layer_digest;
        std::string stage;
        std::string feed;
        std::size_t total_layers;
        uint16_t progress;
        bool complete;
    };
    struct registry_session
    {
        std::string name;
        std::string token;
        int expires_in;
        std::string issued_at;
    };
}

#endif // __DAEMON_CORE_OCI_PAYLOADS__