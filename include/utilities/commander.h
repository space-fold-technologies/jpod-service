#ifndef __JPOD_COMMAND_RUNNER__
#define __JPOD_COMMAND_RUNNER__

#include <pstream.h>
#include <result.h>
#include <spdlog/fmt/fmt.h>
namespace utilties {
  class Runner {
  public:
    static Result<std::string, std::string> execute(const std::string command);
    static Result<std::string, std::string> execute(const std::string, const char args...);
  };
};     // namespace utilties
#endif // __JPOD_COMMAND_RUNNER__