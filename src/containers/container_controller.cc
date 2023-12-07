#include <containers/container_controller.h>
#include <containers/container_service.h>
#include <spdlog/spdlog.h>

namespace containers
{
    ContainerController::ContainerController(std::shared_ptr<ContainerService> service) : service(std::move(service)), logger(spdlog::get("jpod"))
    {
    }
    void ContainerController::on_registration(HandlerRegistry &registry)
    {
        registry.register_handler("container-list", this, &ContainerController::on_list);
        registry.register_handler("container-create", this, &ContainerController::on_list);
        registry.register_handler("container-run", this, &ContainerController::on_run);
        registry.register_handler("container-start", this, &ContainerController::on_start);
        registry.register_handler("container-stop", this, &ContainerController::on_stop);
        registry.register_handler("container-remove", this, &ContainerController::on_remove);
        registry.register_handler("container-log", this, &ContainerController::on_log);
    }
    void ContainerController::on_list(const Request &req, std::shared_ptr<Response> resp)
    {
        logger->info("list command called");
    }
    void ContainerController::on_create(const Request &req, std::shared_ptr<Response> resp)
    {
        logger->info("list command create");
    }
    void ContainerController::on_run(const Request &req, std::shared_ptr<Response> resp)
    {
        logger->info("list command run");
    }
    void ContainerController::on_start(const Request &req, std::shared_ptr<Response> resp)
    {
        logger->info("list command start");
    }
    void ContainerController::on_stop(const Request &req, std::shared_ptr<Response> resp)
    {
        logger->info("list command stop");
    }
    void ContainerController::on_remove(const Request &req, std::shared_ptr<Response> resp)
    {
        logger->info("list command remove");
    }
    void ContainerController::on_log(const Request &req, std::shared_ptr<Response> resp)
    {
        logger->info("list command log");
    }
    ContainerController::~ContainerController()
    {
    }
}