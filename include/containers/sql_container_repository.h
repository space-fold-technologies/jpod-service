#ifndef __JPOD_SERVICE_CONTAINERS_SQL_REPOSITORY__
#define __JPOD_SERVICE_CONTAINERS_SQL_REPOSITORY__

#include <containers/container_repository.h>
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
namespace containers
{
    class SQLContainerRepository : public ContainerRepository
    {
    public:
        explicit SQLContainerRepository(std::shared_ptr<DataSource> data_source);
        virtual ~SQLContainerRepository();
        std::error_code save(const ContainerDetails &details) override;
        std::vector<ContainerSummary> search(const std::string &query) override;
        tl::expected<ContainerDetails, std::error_code> fetch(const std::string &identifier) override;
        std::error_code remove(const std::string &identifier) override;

    private:
        std::shared_ptr<DataSource> data_source;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __JPOD_SERVICE_CONTAINERS_SQL_REPOSITORY__