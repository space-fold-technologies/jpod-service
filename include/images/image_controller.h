#ifndef __JPOD_SERVICE_IMAGES_IMAGE_CONTROLLER__
#define __JPOD_SERVICE_IMAGES_IMAGE_CONTROLLER__

#include <memory>
#include <core/networks/messages/controller.h>

namespace spdlog
{
    class logger;
}

using namespace core::networks::messages;

namespace images
{
    class ImageService;
    class ImageController : public Controller
    {
    public:
        ImageController(std::shared_ptr<ImageService> service);
        virtual ~ImageController();

    private:
        void on_list(const Request &req, std::shared_ptr<Response> resp);
        void on_fetch(const Request &req, std::shared_ptr<Response> resp);
        void on_build(const Request &req, std::shared_ptr<Response> resp);
        void on_remove(const Request &req, std::shared_ptr<Response> resp);
    private:
        std::shared_ptr<ImageService> service;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __JPOD_SERVICE_IMAGES_IMAGE_CONTROLLER__