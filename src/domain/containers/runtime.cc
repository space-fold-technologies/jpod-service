#include <domain/containers/runtime.h>
#include <asio/io_context.hpp>
#if defined(__FreeBSD__)
#include <domain/containers/freebsd/freebsd_container.h>
#endif
#include <spdlog/spdlog.h>
namespace domain::containers
{
    runtime::runtime(asio::io_context &context) : context(context),
                                                  logger(spdlog::get("jpod"))
    {
    }
    void runtime::create_container(operation_details details)
    {
#if defined(__FreeBSD__) // this is probably not going to be that bad right ?
        containers.emplace(details.identifier, std::make_shared<freebsd::freebsd_container>(context, std::move(details), *this));
#endif
        // after all the os specific switching you can initialize the containers
        containers.at(details.identifier)->initialize(); // might have to use something specific to asio like asio::post
    }
    void runtime::container_initialized(const std::string &identifier)
    {
        containers.at(identifier)->start(); // might have to use something specific to asio like asio::post
        // will need to add a container monitor to hold logger information during runtime
    }
    void runtime::container_started(const std::string &identifier)
    {
        logger->info("container: {} started", identifier);
    }
    void runtime::container_failed(const std::string &identifier, const std::error_code &error)
    {
    }
    void runtime::container_stopped(const std::string &identifier)
    {
    }
    void runtime::remove_container(std::string &identifier)
    {
    }
    runtime::~runtime()
    {
    }
}