#include <images/image_service.h>
#include <images/image_repository.h>
#include <images/data_types.h>
#include <spdlog/spdlog.h>

namespace images
{
    ImageService::ImageService(std::shared_ptr<ImageRepository> repository) : repository(repository),
                                                                              logger(spdlog::get("jpod"))
    {
    }
    void ImageService::build_image(const ConstructionDetails &details, std::shared_ptr<Response> response)
    {
        // TODO : create specialized instruction runners that will operate on the system as required
    }
    void ImageService::push_image(const std::string &identifier, const AccessDetails &details, std::shared_ptr<Response> response)
    {
    }
    void ImageService::on_instruction_runner_initialized(std::string id)
    {
    }
    void ImageService::on_instruction_runner_data_received(std::string id, const std::vector<uint8_t> &content)
    {
    }
    void ImageService::on_instruction_runner_completion(std::string id, const std::error_code &err)
    {
    }
    ImageService::~ImageService()
    {
    }
}