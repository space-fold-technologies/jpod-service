#include <domain/containers/freebsd/freebsd_container.h>
#include <domain/containers/container_listener.h>
#include <asio/io_context.hpp>
#include <spdlog/spdlog.h>

namespace domain::containers::freebsd
{
    freebsd_container::freebsd_container(asio::io_context &context, const container_details &details) : context(context),
                                                                                                        details(details),
                                                                                                        logger(spdlog::get("jpod")),
                                                                                                        listener(nullptr)
    {
    }
    void freebsd_container::initialize()
    {
    }
    void freebsd_container::register_listener(std::shared_ptr<container_listener> listener)
    {
        this->listener = std::move(listener);
    }
    freebsd_container::~freebsd_container()
    {
    }
}