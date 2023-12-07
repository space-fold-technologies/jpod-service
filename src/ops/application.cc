#include <ops/application.h>
#include <core/bus/event_bus.h>
#include <ops/events.h>
#include <core/connections/connection.h>
#include <core/connections/ipc/domain_connection_acceptor.h>
#include <containers/container_runtime.h>
#include <containers/container_repository.h>
#include <images/image_repository.h>
#include <spdlog/spdlog.h>

namespace operations
{
    Application::Application(asio::io_context &context,
                             std::shared_ptr<containers::ContainerRuntime> runtime,
                             std::shared_ptr<containers::ContainerRepository> container_repository,
                             std::shared_ptr<images::ImageRepository> image_repository) : context(context),
                                                                                              runtime(runtime),
                                                                                              container_repository(container_repository),
                                                                                              image_repository(image_repository),
                                                                                              domain_connection_acceptor(std::make_unique<ipc::DomainConnectionAcceptor>(context, *this)),
                                                                                              event_bus(std::make_shared<core::bus::EventBus>()),
                                                                                              logger(spdlog::get("jpod"))
    {
        event_bus->register_handler<ShutdownEvent>(
            SHUTDOWN_SESSION,
            [this](const auto &event)
            {
                if (auto pos = this->sessions.find(event.id); pos != this->sessions.end())
                {
                    this->sessions.erase(pos);
                }
            });
    }
    void Application::start()
    {
        domain_connection_acceptor->start();
        logger->info("started acceptor");
    }
    void Application::stop()
    {
        domain_connection_acceptor->stop();
        logger->info("stopped acceptor");
    }
    void Application::connection_accepted(
        std::string identifier,
        Target target,
        std::shared_ptr<Connection> connection)
    {
        // switch (target)
        // {
        // case Target::SHELL:
        //     shells.try_emplace(identifier, std::make_shared<shell::Session>(identifier, context, connection));
        //     break;
        // case Target::LOGS:
        //     break;
        // default:
        //     break;
        // }
    }
    Application::~Application()
    {
        event_bus->remove_handlers();
    }
}