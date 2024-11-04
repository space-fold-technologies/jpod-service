#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_RUN_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_RUN_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <asio/posix/stream_descriptor.hpp>
#include <domain/images/mappings.h>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace spdlog
{
    class logger;
};

namespace asio
{
    class io_context;
};

namespace fs = std::filesystem;
namespace domain::images::instructions
{
    class instruction_listener;
    struct mount_point_entry
    {
        std::string type;
        std::string source;
        fs::path destination;
        int flags;
    };

    class run_instruction : public instruction
    {
        const int WRITE_BUFFER_SIZE = 1024;

    public:
        run_instruction(
            const std::string &identifier,
            const std::string &order,
            asio::io_context &context,
            fs::path &current_directory,
            instruction_listener &listener);
        virtual ~run_instruction();
        void execute() override;

    private:
        std::error_code initialize();
        void disable_stdio_inheritance();
        bool close_on_exec(int fd);
        void clean();
        void read_from_shell();
        bool setup_pipe(int fd);
        // mounting operations
#if defined(__FreeBSD__) || defined(BSD) && !defined(__APPLE__)
        void add_mount_point_entry(std::vector<iovec> &entries, const std::string &key, const std::string &value);
#endif
        bool mount_filesystems(const std::vector<mount_point_entry> &entries, std::error_code &error);
        std::error_code unmount_filesystems(const std::vector<mount_point> &mount_points, fs::path &directory);
        std::vector<mount_point_entry> resolve_mountpoint_folders(const std::vector<mount_point> &entries, std::error_code &error);

    private:
        std::string identifier;
        std::string order;
        asio::io_context &context;
        fs::path &current_directory;
        int file_descriptor;
        pid_t process_identifier;
        std::vector<uint8_t> buffer;
        std::unique_ptr<asio::posix::stream_descriptor> in;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_RUN_INSTRUCTION__