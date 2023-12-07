#include <ops/session.h>
#include <core/connections/connection.h>
#include <core/connections/connection_listener.h>
#include <containers/container_controller.h>
#include <containers/container_service.h>
#include <containers/container_runtime.h>
#include <containers/container_repository.h>
#include <spdlog/spdlog.h>

namespace operations
{
    Session::Session(
        std::shared_ptr<containers::ContainerRuntime> runtime,
        std::shared_ptr<containers::ContainerRepository> repository,
        std::shared_ptr<core::connections::Connection> connection)
        : runtime(runtime),
          repository(repository),
          connection(std::move(connection)),
          controller(nullptr),
          logger(spdlog::get("jpod"))
    {
        connection->register_callback(std::move(std::shared_ptr<core::connections::ConnectionListener>(this)));
        // register controller providers here
        controller_providers.emplace(
            "container",
            [this]() -> std::shared_ptr<Controller>
            {
                auto service = std::make_shared<containers::ContainerService>(this->runtime, this->repository);
                return std::make_shared<containers::ContainerController>(std::move(service));
            });
        connection->connect();
    }

    void Session::on_open()
    {
        // We would need to initiate the terminal here and start initializing anything
        logger->info("session with opened");
    }
    void Session::on_close()
    {
        logger->info("session with closed");
    }
    void Session::on_message(const std::vector<uint8_t> &payload)
    {
        // here you can parse the payload and route it to where it is meant to go
    }

    void Session::on_error(const std::error_code &err)
    {
        logger->error("Session::Err: {}", err.message());
    }
    Session::~Session()
    {
    }
}