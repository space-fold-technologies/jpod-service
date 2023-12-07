#ifndef __JPOD_SERVICE_CONTAINERS_CONTAINER_SERVICE__
#define __JPOD_SERVICE_CONTAINERS_CONTAINER_SERVICE__

#include <tl/expected.hpp>
#include <string>
#include <memory>

namespace core::networks::messages
{
    class Response;
};

using namespace core::networks::messages;

namespace containers
{
    class ContainerRuntime;
    class ContainerRepository;
    class ContainerDetails;
    struct Properties;

    class ContainerService

    {
    public:
        explicit ContainerService(
            std::shared_ptr<ContainerRuntime> runtime,
            std::shared_ptr<ContainerRepository> repository);
        virtual ~ContainerService();
        void logs(const std::string &identifier, std::shared_ptr<Response> response);
        void create(const Properties properties);
        void run(const std::string &name);
        void start(const std::string &name);
        void remove(const std::string &identifier);
        void shell(const std::string &identifier);

    private:
        std::shared_ptr<ContainerRuntime> runtime;
        std::shared_ptr<ContainerRepository> repository;
    };
}

#endif // __JPOD_SERVICE_CONTAINERS_CONTAINER_SERVICE__