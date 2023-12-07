#ifndef __REMOTE_SHELL_SHELL_TERMINAL__
#define __REMOTE_SHELL_SHELL_TERMINAL__

#include <asio/io_context.hpp>
#include <asio/posix/stream_descriptor.hpp>
#include <asio/streambuf.hpp>
#include <fcntl.h>
#include <memory>
#include <string>
#include <tl/expected.hpp>

namespace spdlog
{
  class logger;
};
namespace shell
{
  class TerminalListener;
  struct UserDetails;

  class Terminal
  {
    const long READ_WAIT_SECONDS = 1;
    const long READ_WAIT_MICRO_SECONDS = 500;
    const long WRITE_WAIT_SECONDS = 1;
    const long WRITE_WAIT_MICRO_SECONDS = 500;
    const int WRITE_BUFFER_SIZE = 1024;

  public:
    Terminal(asio::io_context &context, TerminalListener &listener);
    ~Terminal();
    int initialize(const std::string &id, const std::string &username); // meant for freebsd jail as target
    void start();
    void resize(int columns, int rows);
    void write(const std::vector<uint8_t> &data);

  private:
    void run();
    bool close_on_exec(int fd);
    void read_from_shell();
    void disable_stdio_inheritance();
    bool is_available_for_read();
    bool is_available_for_write();
    void process_wait();
    bool setup_pipe(int fd);
    tl::expected<UserDetails, std::string> fetch_user_information(const std::string &username);
    void setup_environment(const UserDetails &details);
    void clean();

  private:
    asio::io_context &context;
    TerminalListener &listener;
    int file_descriptor;
    pid_t process_identifier;
    std::vector<uint8_t> buffer;
    std::unique_ptr<asio::posix::stream_descriptor> in;
    std::unique_ptr<asio::posix::stream_descriptor> out;
    std::shared_ptr<spdlog::logger> logger;
  };
};

#endif // __REMOTE_SHELL_SHELL_TERMINAL__