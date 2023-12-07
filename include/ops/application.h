#ifndef __JPOD_SERVICE_OPERATIONS_APPLICATION__
#define __JPOD_SERVICE_OPERATIONS_APPLICATION__
#include <asio/io_context.hpp>
#include <memory>
#include <core/connections/connection_acceptor_listener.h>
#include <map>

using namespace core::connections;

namespace core::bus
{
    class EventBus;
};
namespace spdlog
{
    class logger;
};
namespace core::connections
{
    class Connection;
};
namespace core::connections::ipc
{
    class DomainConnectionAcceptor;
};
namespace containers
{
    class ContainerRuntime;
    class ContainerRepository;
};
namespace images
{
    class ImageRepository;
};

namespace operations
{
    class Session;
    class Application : public ConnectionAcceptorListener
    {
    public:
        Application(asio::io_context &context,
                    std::shared_ptr<containers::ContainerRuntime> runtime,
                    std::shared_ptr<containers::ContainerRepository> container_repository,
                    std::shared_ptr<images::ImageRepository> image_repository);
        virtual ~Application();
        void start();
        void stop();
        void connection_accepted(
            std::string identifier,
            Target target,
            std::shared_ptr<core::connections::Connection> connection) override;

    private:
        asio::io_context &context;
        std::shared_ptr<containers::ContainerRuntime> runtime;
        std::shared_ptr<containers::ContainerRepository> container_repository;
        std::shared_ptr<images::ImageRepository> image_repository;
        std::unique_ptr<ipc::DomainConnectionAcceptor> domain_connection_acceptor;
        std::map<std::string, std::shared_ptr<Session>> sessions;
        std::shared_ptr<core::bus::EventBus> event_bus;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __JPOD_SERVICE_OPERATIONS_APPLICATION__