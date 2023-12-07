#ifndef __JPOD_SERVICE_IMAGES_RUN_INSTRUCTION__
#define __JPOD_SERVICE_IMAGES_RUN_INSTRUCTION__

#include <images/instruction.h>
#include <fcntl.h>
#include <asio/io_context.hpp>
#include <asio/posix/stream_descriptor.hpp>
#include <asio/streambuf.hpp>
#include <memory>
#include <string>

namespace spdlog
{
    class logger;
};

namespace images
{
    class RunInstruction : public Instruction
    {
        const int WRITE_BUFFER_SIZE = 1024;

    public:
        RunInstruction(asio::io_context &context,
                       const std::string &id,
                       const std::string &source,
                       const std::string &path,
                       InstructionListener &listener);
        virtual ~RunInstruction();
        std::error_code parse() override;
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
        asio::io_context &context;
        const std::string id;
        const std::string &source;
        const std::string& path;
        InstructionListener &listener;
        int file_descriptor;
        pid_t process_identifier;
        std::vector<uint8_t> buffer;
        std::unique_ptr<asio::posix::stream_descriptor> in;
        std::shared_ptr<spdlog::logger> logger;
    };
}

#endif // __JPOD_SERVICE_IMAGES_RUN_INSTRUCTION__