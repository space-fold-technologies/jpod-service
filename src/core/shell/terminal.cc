#include <core/shell/terminal.h>

#include <libutil.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace shell {
  Terminal::Terminal()
      : process_identifier(0) {
  }

  bool Terminal::initialize(const std::string &shell) {
    process_identifier = forkpty(&file_descriptor, NULL, NULL, NULL);
    if (process_identifier < 0) {
      //We failed to fork this process
      return false;
    } else if (execlp(shell.c_str(), shell.c_str(), "-i", NULL) < 0) {
      //We failed to start off run of interactive shell
      return false;
    }
    // set the file descriptor non blocking
    int flags = fcntl(file_descriptor, F_GETFL);
    if (flags != -1) {
      fcntl(file_descriptor, F_SETFD, flags | O_NONBLOCK);
    }
    // set the file descriptor close-on-exec
    return close_on_exec();
  }

  void Terminal::resize(int columns, int rows) {
    struct winsize size;
    size.ws_col = (unsigned short)columns;
    size.ws_row = (unsigned short)rows;
    size.ws_xpixel = 0;
    size.ws_ypixel = 0;
    if (ioctl(file_descriptor, TIOCSWINSZ, &size) < 0) {
      //strerror(errno)
    }
  }

  void shell::Terminal::on_update(READ_CALLBACK read_callback) {
    this->read_callback = read_callback;
  }

  bool Terminal::write_to_shell(const std::string &data) {
    if (write(file_descriptor, data.c_str(), data.size()) != (ssize_t)data.size()) {
      // we have an issue, we did not write all the information we wanted
      // Matheu Butler scared me so i'm also here checking this out as well
      return false;
    }
    return true;
  }

  bool Terminal::is_available_for_write() {
    struct pollfd poll_file_descriptor;
    sigset_t signal_mask;

    poll_file_descriptor.fd = file_descriptor;
    poll_file_descriptor.events = POLLOUT;

    struct timespec time_specification;
    time_specification.tv_sec = WRITE_WAIT_SECONDS;
    time_specification.tv_nsec = WRITE_WAIT_MICRO_SECONDS;
    struct pollfd fds[] = {poll_file_descriptor};

    if (ppoll(fds, 1, &time_specification, &signal_mask) < 0) {
      return false;
    }
    short result = fds[0].revents;
    if (result == POLLOUT || result == POLLWRBAND || result == POLLWRNORM) {
      return true;
    }
    return false;
  }

  void Terminal::run() {
    if (file_descriptor > 0) {
      while (1) {
        if (is_available_for_read()) {
          read_from_shell();
        }
      }
    }
  }

  bool Terminal::close_on_exec() {
    int flags = fcntl(file_descriptor, F_GETFD);
    if (flags < 0) {
      return false;
    }
    return (flags & FD_CLOEXEC) == 0 || fcntl(file_descriptor, F_SETFD, flags | FD_CLOEXEC) != -1;
  }

  bool Terminal::is_available_for_read() {
    struct pollfd poll_file_descriptor;
    sigset_t signal_mask;

    poll_file_descriptor.fd = file_descriptor;
    poll_file_descriptor.events = POLLIN;

    struct timespec time_specification;
    time_specification.tv_sec = READ_WAIT_SECONDS;
    time_specification.tv_nsec = READ_WAIT_MICRO_SECONDS;
    struct pollfd fds[] = {poll_file_descriptor};

    if (ppoll(fds, 1, &time_specification, &signal_mask) < 0) {
      return false;
    }
    short result = fds[0].revents;
    if (result == POLLIN || result == POLLRDBAND || result == POLLRDNORM) {
      return true;
    }
    return false;
  }

  void Terminal::read_from_shell() {
    char buffer[WRITE_BUFFER_SIZE];
    while (read(file_descriptor, buffer, WRITE_BUFFER_SIZE) > 0) {
      // We can use this for debugging as well
      this->read_callback(std::string(buffer));
    }
  }

  Terminal::~Terminal() {
  }

} // namespace shell