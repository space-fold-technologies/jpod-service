#ifndef __JPOD_SERVICE_IMAGES_ERRORS__
#define __JPOD_SERVICE_IMAGES_ERRORS__

#include <system_error>
#include <map>
#include <string>

namespace images
{
    enum class error_code
    {
        file_not_found,
        directory_not_found,
        invalid_origin,
        invalid_destination,
        invalid_copy_instruction
    };
    const inline std::map<error_code, std::string> error_map{
        {error_code::file_not_found, "file not found"},
        {error_code::directory_not_found, "directory not found"},
        {error_code::invalid_origin, "invalid origin"},
        {error_code::invalid_destination, "invalid destination"},
        {error_code::invalid_copy_instruction, "invalid copy instruction"}};

    struct failure_category : public std::error_category
    {
        failure_category() {}
        virtual ~failure_category() = default;
        failure_category(const failure_category &) = delete;
        const char *name() const noexcept override
        {
            return "image failures";
        }

        std::string message(int ec) const override
        {
            static const std::string unknown_error_code("unknown image failure");
            if (auto pos = error_map.find(static_cast<error_code>(ec)); pos != error_map.end())
            {
                return pos->second;
            }
            return unknown_error_code;
        }
    };

    const failure_category& __category()
    {
        static failure_category fc;
        return fc;
    }

    const std::error_code make_error_code(error_code ec) noexcept
    {
        
        return std::error_code{static_cast<int>(ec), __category()};
    };
}

#endif // __JPOD_SERVICE_IMAGES_ERRORS__