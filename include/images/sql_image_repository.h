#ifndef __JPOD_SERVICE_IMAGES_SQL_IMAGE_REPOSITORY__
#define __JPOD_SERVICE_IMAGES_SQL_IMAGE_REPOSITORY__

#include <images/image_repository.h>
#include <memory>

namespace spdlog
{
    class logger;
};
namespace core::databases
{
    class DataSource;
};
using namespace core::databases;
namespace images
{
    class SQLImageRepository : public ImageRepository
    {
    public:
        explicit SQLImageRepository(std::shared_ptr<DataSource> data_source);
        virtual ~SQLImageRepository();
        std::error_code save(const ImageDetails &details) override;
        std::vector<ImageSummary> search(const std::string &query) override;
        tl::expected<ImageDetails, std::error_code> fetch(const std::string &identifier) override;
        bool exists(const std::string &name, const std::string &version) override;
        std::vector<AccessDetails> active_registries() override;
        std::error_code remove(const std::string &identifier) override;

    private:
        std::shared_ptr<DataSource> data_source;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __JPOD_SERVICE_IMAGES_SQL_IMAGE_REPOSITORY__