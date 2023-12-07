#ifndef __JPOD_SERVICE_CONTAINERS_CONTAINER__
#define __JPOD_SERVICE_CONTAINERS_CONTAINER__

#include <asio/io_context.hpp>
#include <asio/posix/stream_descriptor.hpp>
#include <asio/streambuf.hpp>
#include <fcntl.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <tl/expected.hpp>

struct jailparam;
struct iovec;
namespace spdlog
{
    class logger;
};

namespace containers
{
    struct UserDetails;
    struct ContainerDetails;
    class ContainerListener;
    class Container : public std::enable_shared_from_this<Container>
    {
        const int WRITE_BUFFER_SIZE = 1024;

    public:
        Container(asio::io_context &context,
                  const ContainerDetails &details,
                  ContainerListener &listener);
        virtual ~Container();
        void start();
        void stop();
        void resize(int columns, int rows);
        void initialize();

    private:
        int start_process_in_jail();
        void start_jail();
        void clean();
        bool close_on_exec(int fd);
        void read_from_shell();
        void disable_stdio_inheritance();
        void process_wait();
        bool setup_pipe(int fd);
        tl::expected<UserDetails, std::error_code> fetch_user_information(const std::string &username);
        void setup_environment(const UserDetails &details);
        void add_parameter(std::vector<jailparam> &parameters, const std::string &key, const std::string &value);
        void add_mount_point_entry(std::vector<iovec> &entries, const std::string &key, const std::string &value);
        std::error_code mount_filesystems();

    private:
        asio::io_context &context;
        const ContainerDetails &details;
        ContainerListener &listener;
        int file_descriptor;
        pid_t process_identifier;
        std::vector<uint8_t> buffer;
        std::unique_ptr<asio::posix::stream_descriptor> in;
        std::shared_ptr<spdlog::logger> logger;
        std::map<std::string, uint64_t> mounted_points;
    };
}
#endif // __JPOD_SERVICE_CONTAINERS_CONTAINER__