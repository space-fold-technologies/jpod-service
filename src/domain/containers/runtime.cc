#include <domain/containers/runtime.h>
#include <domain/containers/monitor.h>
#include <asio/io_context.hpp>
#if defined(__FreeBSD__)
#include <domain/containers/freebsd/freebsd_container.h>
#endif
#include <spdlog/spdlog.h>
namespace domain::containers
{
    runtime::runtime(asio::io_context &context, monitor_provider container_monitor_provider) : context(context),
                                                                                               container_monitor_provider(container_monitor_provider),
                                                                                               logger(spdlog::get("jpod"))
    {
    }
    void runtime::create_container(operation_details details)
    {
#if defined(__FreeBSD__) // this is probably not going to be that bad right ?
        containers.emplace(details.identifier, std::make_shared<freebsd::freebsd_container>(context, std::move(details), *this));
#endif
        containers.at(details.identifier)->initialize(); // might have to use something specific to asio like asio::post
    }
    void runtime::container_initialized(const std::string &identifier)
    {
        if (monitors.find(identifier) == monitors.end())
        {
            monitors.emplace(identifier, container_monitor_provider());
        }
        auto monitor = monitors.at(identifier);
        auto container = containers.at(identifier); // might have to use something specific to asio like asio::post
        container->register_listener(monitor);
        container->start();
    }
    void runtime::container_started(const std::string &identifier)
    {
        logger->info("container: {} started", identifier);
    }
    void runtime::container_failed(const std::string &identifier, const std::error_code &error)
    {
        logger->info("container: {} failed", identifier);
    }
    void runtime::container_stopped(const std::string &identifier)
    {
        logger->info("container: {} stopped", identifier);
    }
    void runtime::remove_container(std::string &identifier)
    {
        if (auto pos = containers.find(identifier); pos != containers.end())
        {
            containers.erase(pos);
        }

        if (auto pos = monitors.find(identifier); pos != monitors.end())
        {
            monitors.erase(pos);
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