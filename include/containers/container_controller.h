#ifndef __JPOD_SERVICE_CONTAINERS_CONTAINER_CONTROLLER__
#define __JPOD_SERVICE_CONTAINERS_CONTAINER_CONTROLLER__
#include <memory>
#include <core/networks/messages/controller.h>

namespace spdlog
{
    class logger;
}

using namespace core::networks::messages;

namespace containers
{
    class ContainerService;
    class ContainerController : public Controller
    {
    public:
        ContainerController(std::shared_ptr<ContainerService> service);
        virtual ~ContainerController();
        void on_registration(HandlerRegistry &registry) override;

    private:
        void on_list(const Request &req, std::shared_ptr<Response> resp);
        void on_create(const Request &req, std::shared_ptr<Response> resp);
        void on_run(const Request &req, std::shared_ptr<Response> resp);
        void on_start(const Request &req, std::shared_ptr<Response> resp);
        void on_stop(const Request &req, std::shared_ptr<Response> resp);
        void on_remove(const Request &req, std::shared_ptr<Response> resp);
        void on_log(const Request &req, std::shared_ptr<Response> resp);

    private:
        std::shared_ptr<ContainerService> service;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __JPOD_SERVICE_CONTAINERS_CONTAINER_CONTROLLER__