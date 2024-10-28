#ifndef __DAEMON_CORE_OCI_LAYER_ERRORS__
#define __DAEMON_CORE_OCI_LAYER_ERRORS__

#include <system_error>
#include <string>
#include <map>

namespace core::oci
{
    enum class layer_error_codes
    {
        file_exists,
        folder_exists,
        file_creation_failed,
        folder_creation_failed,
        no_file_or_directory_found
    };

    const inline std::map<layer_error_codes, std::string> layer_error_map
    {

    };

    struct layer_failure_category : public std::error_category
    {
        layer_failure_category() {}
        virtual ~layer_failure_category() = default;
        layer_failure_category(const layer_failure_category &) = delete;
        const char *name() const noexcept override
        {
            return "oci layer failures";
        }

        std::string message(int ec) const override
        {
            static const std::string unknown_error_code("unknown oci layer failure");
            if (auto pos = layer_error_map.find(static_cast<layer_error_codes>(ec)); pos != layer_error_map.end())
            {
                return pos->second;
            }
            return unknown_error_code;
        }
    };

    inline const layer_failure_category &__layer_failure_category()
    {
        static layer_failure_category fc;
        return fc;
    }

    inline const std::error_code make_layer_error_code(layer_error_codes ec) noexcept
    {

        return std::error_code{static_cast<int>(ec), __layer_failure_category()};
    };
}


#endif //__DAEMON_CORE_OCI_LAYER_ERRORS__