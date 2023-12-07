#ifndef __JPOD_SERVICE_IMAGES_IMAGE_SERVICE__
#define __JPOD_SERVICE_IMAGES_IMAGE_SERVICE__

#include <images/instruction_listener.h>
#include <memory>
#include <map>

namespace spdlog
{
    class logger;
};
namespace core::networks::messages
{
    class Response;
};

using namespace core::networks::messages;

namespace images
{
    class ImageRepository;
    class InstructionRunner;
    class ImageDetails;
    class AccessDetails;
    class MountPoint;
    class ConstructionDetails;
    class ImageService : public InstructionListener
    {
    public:
        explicit ImageService(std::shared_ptr<ImageRepository> repository);
        virtual ~ImageService();
        void build_image(const ConstructionDetails &details, std::shared_ptr<Response> response);
        void push_image(const std::string &identifier, const AccessDetails &details, std::shared_ptr<Response> response);
        void on_instruction_runner_initialized(std::string id) override;
        void on_instruction_runner_data_received(std::string id, const std::vector<uint8_t> &content) override;
        void on_instruction_runner_completion(std::string id, const std::error_code &err) override;

    private:
        void download_to_local_storage(const std::string &identifier, const AccessDetails &details, std::shared_ptr<Response> response);
        std::error_code mount_filesystems(const std::string &parent_folder, const std::vector<MountPoint> &mount_points);

    private:
        std::shared_ptr<ImageRepository> repository;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __JPOD_SERVICE_IMAGES_IMAGE_SERVICE__