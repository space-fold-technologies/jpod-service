#include <containers/container_service.h>
#include <containers/container_runtime.h>
#include <containers/container_repository.h>
#include <core/networks/messages/response.h>
#include <containers/data_types.h>

namespace containers
{
    ContainerService::ContainerService(
        std::shared_ptr<ContainerRuntime> runtime,
        std::shared_ptr<ContainerRepository> repository) : runtime(runtime),
                                                           repository(repository)
    {
    }
    void ContainerService::logs(const std::string &identifier, std::shared_ptr<Response> response)
    {
    }
    void ContainerService::create(const Properties properties)
    {
    }
    void ContainerService::run(const std::string &name)
    {
    }
    void ContainerService::start(const std::string &name)
    {
    }
    void ContainerService::remove(const std::string &identifier)
    {
    }
    void ContainerService::shell(const std::string &identifier)
    {
    }
    ContainerService::~ContainerService()
    {
    }
}