#ifndef __JPOD_SERVICE_CONTAINERS_REPOSITORY__
#define __JPOD_SERVICE_CONTAINERS_REPOSITORY__

#include <system_error>
#include <containers/data_types.h>
#include <tl/expected.hpp>
#include <vector>

namespace containers
{
    class ContainerRepository
    {
    public:
        virtual std::error_code save(const ContainerDetails &details) = 0;
        virtual std::vector<ContainerSummary> search(const std::string &query) = 0;
        virtual tl::expected<ContainerDetails, std::error_code> fetch(const std::string &identifier) = 0;
        virtual std::error_code remove(const std::string &identifier) = 0;
    };
}
#endif // __JPOD_SERVICE_CONTAINERS_REPOSITORY__