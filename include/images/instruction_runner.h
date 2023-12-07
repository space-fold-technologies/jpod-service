#ifndef __JPOD_SERVICE_IMAGES_INSTRUCTION_RUNNER__
#define __JPOD_SERVICE_IMAGES_INSTRUCTION_RUNNER__
#include <fcntl.h>
#include <asio/io_context.hpp>
#include <asio/posix/stream_descriptor.hpp>
#include <asio/streambuf.hpp>
#include <memory>
#include <string>
#include <system_error>
/*
 // Properties properties{};
  // properties.snapshot_path = "/home/william/images/alpine@latest.gz";
  // properties.operations.push_back("apk update && apk add gcc \n");
  // properties.operations.push_back("uname \n");
  // properties.operations.push_back("exit \n");
  // properties.mount_points.push_back(MountPoint{"devfs", "dev", "rw", 0});
  // properties.mount_points.push_back(MountPoint{"linprocfs", "proc", "rw", 0});
  // properties.mount_points.push_back(MountPoint{"linsysfs", "sys", "rw", 0});
  // properties.mount_points.push_back(MountPoint{"tmpfs", "dev/shm", "rw,mode=1777", 0});

*/
namespace spdlog
{
    class logger;
}
namespace images
{
    class InstructionListener;
    class InstructionRunner
    {
        const int WRITE_BUFFER_SIZE = 1024;

    public:
        InstructionRunner(
            asio::io_context &context,
            std::string &id,
            std::string &instruction,
            std::string &path,
            InstructionListener &listener);
        virtual ~InstructionRunner();
        void initialize();
        void run();

    private:
        void disable_stdio_inheritance();
        bool close_on_exec(int fd);
        void clean();
        void read_from_shell();
        bool setup_pipe(int fd);
        void process_wait();

    private:
        asio::io_context &context;
        std::string id;
        std::string instruction;
        std::string path;
        InstructionListener &listener;
        int file_descriptor;
        pid_t process_identifier;
        std::vector<uint8_t> buffer;
        std::unique_ptr<asio::posix::stream_descriptor> in;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __JPOD_SERVICE_IMAGES_INSTRUCTION_RUNNER__