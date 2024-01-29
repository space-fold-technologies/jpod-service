#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER__

#include <domain/containers/container.h>
#include <memory>
#include <optional>

namespace asio
{
    class io_context;
}

namespace spdlog
{
    class logger;
}

namespace domain::containers
{
    class container_listener;
}

struct jailparam;

namespace domain::containers::freebsd
{

    class freebsd_container : public container
    {
        const int WRITE_BUFFER_SIZE = 1024;
        struct user_details;

    public:
        freebsd_container(asio::io_context &context, const container_details &details);
        virtual ~freebsd_container();
        void initialize() override;
        void register_listener(std::shared_ptr<container_listener> listener) override;

    private:
        int start_process_in_jail();
        void start_jail();
        void clean();
        bool close_on_exec(int fd);
        void read_from_shell();
        void disable_stdio_inheritance();
        void process_wait();
        bool setup_pipe(int fd);
        // std::optional<user_details> fetch_user_information(const std::string &username, std::error_code &error); // this will be put into a utility
        // void setup_environment(const user_details &details); // into utility class
        // void add_parameter(std::vector<jailparam> &parameters, const std::string &key, const std::string &value); // into utility class
        // void add_mount_point_entry(std::vector<iovec> &entries, const std::string &key, const std::string &value);

    private:
        asio::io_context &context;
        const container_details &details;
        std::shared_ptr<spdlog::logger> logger;
        std::shared_ptr<container_listener> listener;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_FREEBSD_CONTAINER__