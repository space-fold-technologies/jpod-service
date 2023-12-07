#include <images/sql_image_repository.h>
#include <core/databases/data_source.h>
#include <spdlog/spdlog.h>

namespace images
{
    SQLImageRepository::SQLImageRepository(std::shared_ptr<DataSource> data_source) : data_source(data_source),
                                                                                      logger(spdlog::get("jpod"))
    {
    }
    std::error_code SQLImageRepository::save(const ImageDetails &details)
    {
        return std::make_error_code(std::errc::io_error);
    }
    std::vector<ImageSummary> SQLImageRepository::search(const std::string &query)
    {
    }
    tl::expected<ImageDetails, std::error_code> SQLImageRepository::fetch(const std::string &identifier)
    {
    }
    bool SQLImageRepository::exists(const std::string &name, const std::string &version)
    {
        return false;
    }
    std::vector<AccessDetails> SQLImageRepository::active_registries()
    {
        return std::vector<AccessDetails>{};
    }
    std::error_code SQLImageRepository::remove(const std::string &identifier)
    {
        return std::make_error_code(std::errc::io_error);
    }
    SQLImageRepository::~SQLImageRepository()
    {
    }
}