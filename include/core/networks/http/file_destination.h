#ifndef __JPOD_SERVICE_CORE_NETWORKS_HTTP_FILE_DESTINATION__
#define __JPOD_SERVICE_CORE_NETWORKS_HTTP_FILE_DESTINATION__

#include <core/networks/http/download_components.h>
#include <memory>

namespace spdlog
{
    class logger;
};

namespace core::networks::http
{
    constexpr std::size_t DEFAULT_BUFFER = 1024000;
    class FileDestination : public Destination
    {
    public:
        FileDestination(const std::string file_path, std::size_t buffer_size = DEFAULT_BUFFER);
        ~FileDestination();
        bool is_valid() override;
        size_t chunk_size() const override;
        size_t write(const std::vector<uint8_t> &data) override;

    private:
        std::string file_path;
        std::size_t buffer_size;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __JPOD_SERVICE_CORE_NETWORKS_HTTP_FILE_DESTINATION__