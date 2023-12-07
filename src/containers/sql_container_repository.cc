#include <containers/sql_container_repository.h>
#include <core/databases/data_source.h>
#include <spdlog/spdlog.h>

namespace containers
{
    SQLContainerRepository::SQLContainerRepository(std::shared_ptr<DataSource> data_source) : data_source(data_source),
                                                                                              logger(spdlog::get("jpod"))
    {
        logger->info("sql-container-repository loaded!!!");
    }
    std::error_code SQLContainerRepository::save(const ContainerDetails &details)
    {
        return std::make_error_code(std::errc::io_error);
    }
    std::vector<ContainerSummary> SQLContainerRepository::search(const std::string &query)
    {
    }
    tl::expected<ContainerDetails, std::error_code> SQLContainerRepository::fetch(const std::string &identifier)
    {
        return tl::make_unexpected(std::make_error_code(std::errc::io_error));
    }
    std::error_code SQLContainerRepository::remove(const std::string &identifier)
    {
        return std::make_error_code(std::errc::io_error);
    }
    SQLContainerRepository::~SQLContainerRepository()
    {
    }
}