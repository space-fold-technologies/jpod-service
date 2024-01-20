#ifndef __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_RUN_INSTRUCTION__
#define __DAEMON_DOMAIN_IMAGES_INSTRUCTIONS_RUN_INSTRUCTION__

#include <domain/images/instructions/instruction.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>
#include <asio/posix/stream_descriptor.hpp>

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
        void initialize();
        void disable_stdio_inheritance();
        bool close_on_exec(int fd);
        void clean();
        void read_from_shell();
        bool setup_pipe(int fd);
        void process_wait();

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