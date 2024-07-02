#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER_IMPLEMENTATION__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER_IMPLEMENTATION__

#include <domain/containers/container.h>
#include <asio/posix/stream_descriptor.hpp>
#include <asio/streambuf.hpp>
#include <system_error>
#include <memory>
#include <map>

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
        freebsd_container(
            asio::io_context &context,
            operation_details details,
            runtime_listener &listener);
        void initialize() override;
        virtual ~freebsd_container() override;
        void start() override;
        void register_listener(std::shared_ptr<container_listener> operation_listener) override;
        void update_parameters(const std::map<std::string, std::string>& parameters) override;

    private:
        std::error_code mount_file_systems();
        std::error_code unmount_file_systems();
        std::error_code create_jail();
        std::error_code start_process_in_jail();
        bool setup_pipe(int fd);
        void clean();
        void read_from_shell();
        void wait_to_read_from_shell();
        void on_operation_failure(std::error_code &error);
        void on_operation_output(const std::vector<uint8_t> &content);

    private:
        asio::io_context &context;
        int file_descriptor;
        pid_t process_identifier;
        std::vector<uint8_t> buffer;
        std::unique_ptr<asio::posix::stream_descriptor> stream;
        std::shared_ptr<spdlog::logger> logger;
        std::string network;
        std::map<listener_category, std::weak_ptr<container_listener>> operation_listeners;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER_IMPLEMENTATION__