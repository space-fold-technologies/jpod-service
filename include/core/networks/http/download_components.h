#ifndef __JPOD_SERVICE_CORE_NETWORKS_HTTP_DOWNLOAD_COMPONENTS__
#define __JPOD_SERVICE_CORE_NETWORKS_HTTP_DOWNLOAD_COMPONENTS__

#include <cstdint>
#include <string>
#include <vector>
#include <system_error>

namespace core::networks::http
{
    class Destination
    {
    public:
        virtual bool is_valid() = 0;
        virtual size_t chunk_size() const = 0;
        virtual size_t write(const std::vector<uint8_t> &data) = 0;
        virtual ~Destination() = default;
    };

    struct Status
    {
        std::size_t current;
        std::size_t start;
        std::size_t end;
        std::size_t total;
        std::string unit;
        bool complete;
        std::error_code err;
    };

    inline auto failed(std::error_code err) -> Status
    {
        Status status{};
        status.err = err;
        return status;
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_HTTP_DOWNLOAD_COMPONENTS__