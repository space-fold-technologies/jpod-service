#ifndef __DAEMON_DOMAIN_CONTAINERS_FREEBSD_VIRTUAL_TERMINAL__
#define __DAEMON_DOMAIN_CONTAINERS_FREEBSD_VIRTUAL_TERMINAL__

#include <domain/containers/virtual_terminal.h>
#include <domain/containers/terminal_details.h>
#include <asio/posix/stream_descriptor.hpp>
#include <string>
namespace asio
{
    class io_context;
};

namespace spdlog
{
    class logger;
};

namespace domain::containers
{
    class terminal_listener;
};

namespace domain::containers::freebsd
{
    class freebsd_terminal : public virtual_terminal
    {
        const int WRITE_BUFFER_SIZE = 1024;

    public:
        freebsd_terminal(asio::io_context &context,
                         terminal_properties properties,
                         terminal_listener &listener);
        virtual ~freebsd_terminal();
        std::error_code initialize() override;
        void start() override;
        void resize(uint32_t columns, uint32_t rows) override;
        void write(const std::vector<uint8_t> &content) override;

    private:
        void read_from_shell();
        void wait_to_read_from_shell();
        bool setup_pipe(int fd);
        void clean();

    private:
        asio::io_context &context;
        terminal_properties properties;
        terminal_listener &listener;
        int file_descriptor;
        pid_t process_identifier;
        std::vector<uint8_t> buffer;
        std::unique_ptr<asio::posix::stream_descriptor> in;
        std::unique_ptr<asio::posix::stream_descriptor> out;
        std::shared_ptr<spdlog::logger> logger;
    };
}
#endif // __DAEMON_DOMAIN_CONTAINERS_FREEBSD_VIRTUAL_TERMINAL__