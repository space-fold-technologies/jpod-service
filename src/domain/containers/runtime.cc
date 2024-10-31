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
    runtime::runtime(
        asio::io_context &context,
        std::shared_ptr<container_repository> repository,
        monitor_provider container_monitor_provider,
        address_assigner container_address_assigner,
        address_cleaner container_address_cleaner) : context(context),
                                                     repository(repository),
                                                     container_monitor_provider(container_monitor_provider),
                                                     container_address_assigner(container_address_assigner),
                                                     container_address_cleaner(container_address_cleaner),
                                                     logger(spdlog::get("jpod"))
    {
    }
    void runtime::create_container(operation_details details)
    {
        if (details.identifier.empty())
        {
            return;
        }
        auto key = std::string(details.identifier);
#if defined(__FreeBSD__) // this is probably not going to be that bad right ?
        containers.try_emplace(key, std::make_shared<freebsd::freebsd_container>(context, std::move(details), *this));
#endif
        auto container = containers.at(key);
        monitors.try_emplace(key, container_monitor_provider());
        container->register_listener(monitors.at(key));
        container->initialize(); // might have to use something specific to asio like asio::post
    }
    void runtime::container_initialized(const std::string &identifier, const std::string &network)
    {
        if (auto pos = containers.find(identifier); pos != containers.end())
        {   
            auto container = pos->second;
            // now go ahead and assign a network address to the container
            if (auto error = container_address_assigner(identifier, network); !error)
            {
                container->start();
            }
            else
            {
                remove_container(identifier);
                logger->error("container address assignment failed: {}", error.message());
            }
        }
    }
    void runtime::container_started(const std::string &identifier)
    {
        repository->register_status(identifier, "active");
    }
    void runtime::container_failed(const std::string &identifier, const std::error_code &error)
    {
        logger->error("container: {} failed", identifier);
    }
    void runtime::container_stopped(const std::string &identifier, const std::string &network)
    {
        repository->register_status(identifier, "shutdown");
        if(auto error = container_address_cleaner(identifier, network); error)
        {
            logger->error("container-network id[{}]: {}", identifier, error.message());
        }
    }
    void runtime::remove_container(const std::string &identifier)
    {
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
        if (auto pos = containers.find(identifier); pos != containers.end())
        {
            return pos->second;
        }
        return {};
    }
    runtime::~runtime()
    {
        containers.clear();
        monitors.clear();
    }
}