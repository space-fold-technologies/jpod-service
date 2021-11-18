#ifndef __JPOD__TERMINAL__
#define __JPOD__TERMINAL__

#include <fcntl.h>
#include <functional>
#include <string>

namespace shell {
  typedef std::function<void(const std::string &data)> READ_CALLBACK;
  class Terminal {
    const long READ_WAIT_SECONDS = 1;
    const long READ_WAIT_MICRO_SECONDS = 500;
    const long WRITE_WAIT_SECONDS = 1;
    const long WRITE_WAIT_MICRO_SECONDS = 500;
    const int WRITE_BUFFER_SIZE = 1024;

  public:
    Terminal();
    ~Terminal();
    bool initialize(const std::string &shell);
    void resize(int columns, int rows);
    void on_update(READ_CALLBACK read_callback);
    bool write_to_shell(const std::string &data);
    bool is_available_for_write();
    void run();

  private:
    bool close_on_exec();
    void read_from_shell();
    bool is_available_for_read();
    int file_descriptor;
    pid_t process_identifier;
    READ_CALLBACK read_callback;
  };
}; // namespace shell

#endif // __JPOD__TERMINAL__