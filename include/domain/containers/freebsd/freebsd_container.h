#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER_IMPLEMENTATION__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER_IMPLEMENTATION__

#include <domain/containers/container.h>
#include <asio/posix/stream_descriptor.hpp>
#include <asio/streambuf.hpp>
#include <system_error>
#include <memory>

namespace spdlog
{
    class logger;
};

namespace asio
{
    class io_context;
};

namespace domain::containers::freebsd
{
    class freebsd_container : public container
    {
        const int WRITE_BUFFER_SIZE = 1024;

    public:
        freebsd_container(asio::io_context &context, container_details details);
        void initialize() override;
        virtual ~freebsd_container() override;
        void start() override;
        void resize(int columns, int rows) override;
        void register_listener(std::shared_ptr<container_listener> listener) override;

    private:
        std::error_code mount_file_systems();
        std::error_code unmount_file_systems();

    private:
        asio::io_context &context;
        int file_descriptor;
        pid_t process_identifier;
        std::vector<uint8_t> buffer;
        asio::posix::stream_descriptor stream;
        std::shared_ptr<spdlog::logger> logger;
        std::map<std::string, uint64_t> mounted_points;
        std::shared_ptr<container_listener> listener;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER_IMPLEMENTATION__