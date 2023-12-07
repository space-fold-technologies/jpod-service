#ifndef __JPOD_SERVICE_IMAGES_IMAGE_REPOSITORY__
#define __JPOD_SERVICE_IMAGES_IMAGE_REPOSITORY__

#include <vector>
#include <images/data_types.h>
#include <system_error>
#include <tl/expected.hpp>

namespace images
{
    class ImageRepository
    {
    public:
        virtual std::error_code save(const ImageDetails &details) = 0;
        virtual std::vector<ImageSummary> search(const std::string &query) = 0;
        virtual tl::expected<ImageDetails, std::error_code> fetch(const std::string &identifier) = 0;
        virtual bool exists(const std::string &name, const std::string &version) = 0;
        virtual std::vector<AccessDetails> active_registries() = 0;
        virtual std::error_code remove(const std::string &identifier) = 0;
    };
}

#endif // __JPOD_SERVICE_IMAGES_IMAGE_REPOSITORY__