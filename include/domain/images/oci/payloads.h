#ifndef __DAEMON_DOMAIN_IMAGES_OCI_PAYLOADS__
#define __DAEMON_DOMAIN_IMAGES_OCI_PAYLOADS__

#include <nlohmann/json.hpp>

namespace domain::images::oci::payloads
{
    struct manifest_index
    {
        std::string digest;
        std::string media_type;
        std::size_t size;
        std::map<std::string, std::string> platform;
        std::map<std::string, std::string> annotations;
    };

    struct descriptor
    {
        std::string media_type;
        std::size_t size;
        std::string digest;
    };

    struct manifest
    {
        uint32_t schema_version;
        std::string media_type;
        descriptor config;
        std::vector<descriptor> layers;
    };
}
#endif // __DAEMON_DOMAIN_IMAGES_OCI_PAYLOADS__