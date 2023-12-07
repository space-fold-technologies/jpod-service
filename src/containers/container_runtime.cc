#include <containers/container_runtime.h>
#include <containers/data_types.h>
#include <containers/container.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>
#include <thread>

namespace containers
{
    ContainerRuntime::ContainerRuntime(asio::io_context &context) : context(context),
                                                                    running_containers{},
                                                                    logger(spdlog::get("jpod"))
    {
    }
    bool ContainerRuntime::exists(const std::string &id)
    {
        return running_containers.find(id) != running_containers.end();
    }
    void ContainerRuntime::add_container(const ContainerDetails &details)
    {
        auto container = std::make_shared<Container>(context, details, *this);
        running_containers.try_emplace(details.id, container);
        std::thread t([container]()
                      { container->initialize(); });
        t.join();
    }
    void ContainerRuntime::on_container_initialized(const std::string &id)
    {
        if (auto pos = running_containers.find(id); pos != running_containers.end())
        {
            auto container = pos->second;
            std::thread t([container]()
                          { container->start(); });
            t.join();
        }
    }
    void ContainerRuntime::on_container_data_received(const std::string &id, const std::vector<uint8_t> &content)
    {
        fmt::print("{}",std::string(content.begin(), content.end()));
    }
    void ContainerRuntime::on_container_error(const std::string &id, const std::error_code &err)
    {
        logger->error("container with ID: {} err: {}", id, err.message());
        if (auto pos = running_containers.find(id); pos != running_containers.end())
        {
            pos->second->stop();
            running_containers.erase(pos);
        }
    }
    void ContainerRuntime::on_container_shutdown(const std::string &id)
    {
        logger->info("container with ID: {} shutdown", id);
        if (auto pos = running_containers.find(id); pos != running_containers.end())
        {
            pos->second->stop();
            running_containers.erase(pos);
        }
    }
    ContainerRuntime::~ContainerRuntime()
    {
    }
}