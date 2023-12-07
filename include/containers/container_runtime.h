#ifndef __JPOD_SERVICE_CONTAINERS_RUNTIME__
#define __JPOD_SERVICE_CONTAINERS_RUNTIME__

#include <asio/io_context.hpp>
#include <containers/container_listener.h>
#include <memory>
#include <map>

namespace spdlog
{
    class logger;
};

namespace containers
{
    class Container;
    struct ContainerDetails;
    class ContainerRuntime : public ContainerListener
    {

    public:
        ContainerRuntime(asio::io_context &context);
        virtual ~ContainerRuntime();
        bool exists(const std::string &id);
        void add_container(const ContainerDetails &details);
        // callbacks
        void on_container_initialized(const std::string &id) override;
        void on_container_data_received(const std::string &id, const std::vector<uint8_t> &content) override;
        void on_container_error(const std::string &id, const std::error_code &err) override;
        void on_container_shutdown(const std::string &id) override;

    private:
        asio::io_context &context;
        std::map<std::string, std::shared_ptr<Container>> running_containers;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __JPOD_SERVICE_CONTAINERS_RUNTIME__