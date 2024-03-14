#include <domain/containers/runtime.h>
#include <domain/containers/monitor.h>
#include <domain/containers/repository.h>
#include <asio/io_context.hpp>
#if defined(__FreeBSD__)
#include <domain/containers/freebsd/freebsd_container.h>
#endif
#include <spdlog/spdlog.h>
namespace domain::containers
{
    runtime::runtime(asio::io_context &context, std::shared_ptr<container_repository> repository, monitor_provider container_monitor_provider) : context(context),
                                                                                                                                                 repository(repository),
                                                                                                                                                 container_monitor_provider(container_monitor_provider),
                                                                                                                                                 logger(spdlog::get("jpod"))
    {
    }
    void runtime::create_container(operation_details details)
    {
        if (details.identifier.empty())
        {
            return;
        }
        logger->info("adding container: {}", details.identifier);
        auto key = std::string(details.identifier);
#if defined(__FreeBSD__) // this is probably not going to be that bad right ?
        containers.emplace(key, std::make_shared<freebsd::freebsd_container>(context, std::move(details), *this));
#endif
        containers.at(key)->initialize(); // might have to use something specific to asio like asio::post
    }
    void runtime::container_initialized(const std::string &identifier)
    {
        if (monitors.find(identifier) == monitors.end())
        {
            monitors.emplace(identifier, container_monitor_provider());
        }
        if (auto pos = containers.find(identifier); pos != containers.end())
        {
            auto container = pos->second;
            auto monitor = monitors.at(identifier);
            container->register_listener(monitor);
            container->start();
        }
    }
    void runtime::container_started(const std::string &identifier)
    {
        logger->info("container: {} started", identifier);
        repository->register_status(identifier, "active");
    }
    void runtime::container_failed(const std::string &identifier, const std::error_code &error)
    {
        logger->info("container: {} failed", identifier);
    }
    void runtime::container_stopped(const std::string &identifier)
    {
        logger->info("container: {} stopped", identifier);
        repository->register_status(identifier, "shutdown");
    }
    void runtime::remove_container(std::string &identifier)
    {
        for (const auto &entry : containers)
        {
            logger->info("container-id: {}", entry.first);
        }
        if (auto pos = containers.find(identifier); pos != containers.end())
        {
            if (auto pos = monitors.find(identifier); pos != monitors.end())
            {
                monitors.erase(pos);
            }
            containers.erase(pos);
        }
        else
        {
            logger->error("no such container found");
        }
    }
    std::shared_ptr<container> runtime::fetch_container(const std::string &identifier)
    {
        return containers.at(identifier);
    }
    runtime::~runtime()
    {
        containers.clear();
        monitors.clear();
    }
}