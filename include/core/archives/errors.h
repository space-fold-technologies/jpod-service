#ifndef __DAEMON_CORE_ARCHIVES_ERRORS__
#define __DAEMON_CORE_ARCHIVES_ERRORS__

#include <map>
#include <string>
#include <archive.h>
#include <system_error>

namespace core::archives
{
    inline std::map<int, std::string> compression_error_map =
        {
            {ARCHIVE_EOF, "Found end of archive"},
            {ARCHIVE_RETRY, "Retry might succeed"},
            {ARCHIVE_WARN, "Partial Success"},
            {ARCHIVE_FAILED, "Current operation cannot complete"},
            {ARCHIVE_FATAL, "No more operations are possible"}
        };
    struct compression_failure_category : public std::error_category
    {
        compression_failure_category() {}
        virtual ~compression_failure_category() = default;
        compression_failure_category(const compression_failure_category &) = delete;
        const char *name() const noexcept override
        {
            return "compression failures";
        }

        std::string message(int ec) const override
        {
            static const std::string unknown_error_code("unknown compression failure");
            if (auto pos = compression_error_map.find(ec); pos != compression_error_map.end())
            {
                return pos->second;
            }
            return unknown_error_code;
        }
    };

    inline const compression_failure_category &__compression_failure_category()
    {
        static compression_failure_category fc;
        return fc;
    }

    inline const std::error_code make_compression_error_code(int ec) noexcept
    {

        return std::error_code{ec, __compression_failure_category()};
    };
}

#endif // __DAEMON_CORE_ARCHIVES_ERRORS__