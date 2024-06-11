#ifndef __DAEMON_CORE_HTTP_DOWNLOAD_DESTINATION__
#define __DAEMON_CORE_HTTP_DOWNLOAD_DESTINATION__

#include <vector>
#include <cstdint>

namespace core::http
{
    class download_destination
    {
    public:
        virtual ~download_destination() = default;
        virtual bool is_valid() = 0;
        virtual std::size_t chunk_size() const = 0;
        virtual std::size_t write(const std::vector<uint8_t> &data) = 0;
    };
}

#endif // __DAEMON_CORE_HTTP_DOWNLOAD_DESTINATION__