#ifndef __JPOD_SERVICE_OPERATIONS_SESSION__
#define __JPOD_SERVICE_OPERATIONS_SESSION__

#include <asio/io_context.hpp>
#include <core/connections/connection_listener.h>
#include <map>
#include <functional>

namespace spdlog
{
    class logger;
};

namespace core::connections
{
    class Connection;
};

namespace containers
{
    class ContainerRuntime;
    class ContainerRepository;
};

namespace core::networks::messages
{
    class Controller;
};
using namespace core::networks::messages;
namespace operations
{
    typedef std::function<std::shared_ptr<Controller>()> controller_provider;
    class Session : public core::connections::ConnectionListener, public std::enable_shared_from_this<Session>
    {
    public:
        Session(
            std::shared_ptr<containers::ContainerRuntime> runtime,
            std::shared_ptr<containers::ContainerRepository> repository,
            std::shared_ptr<core::connections::Connection> connection);
        virtual ~Session();
        void on_open() override;
        void on_close() override;
        void on_message(const std::vector<uint8_t> &payload) override;
        void on_error(const std::error_code &ec) override;

    private:
        std::shared_ptr<containers::ContainerRuntime> runtime;
        std::shared_ptr<containers::ContainerRepository> repository;
        std::shared_ptr<core::connections::Connection> connection;
        std::shared_ptr<Controller> controller;
        std::shared_ptr<spdlog::logger> logger;
        std::map<std::string, controller_provider> controller_providers;
    };
}
#endif // __JPOD_SERVICE_OPERATIONS_SESSION__