#include <filesystem>
#include <pstream.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>
#include <zfs/zfs_utilities.h>
namespace fs = std::filesystem;

namespace zfs {
  handler::handler(const std::string &pool_name, const std::string &mount_point, const std::string &mount_name)
      : pool_name(pool_name), mount_point(mount_point), mount_name(mount_name) {
  }

  void handler::startup() {
    auto paths = std::vector<std::string>{mount_point, mount_point + "/images", mount_point + "/volumes", mount_point + "/jails"};
    std::for_each(paths.begin(), paths.end(), [](const auto &target_path) {
      fs::path dir(target_path);
      if (!fs::exists(dir)) {
        fs::create_directories(dir);
      }
    });
    fs::permissions(fs::path(mount_point), fs::perms::owner_read | fs::perms::owner_write, fs::perm_options::add);
    if (!exists(fmt::format("{}/{}", pool_name, mount_name))) {
      exec(fmt::format("zfs create -o canmount={} mountpoint={} {}/{}", "off", mount_point, pool_name, mount_name));
    }
    if (!exists(fmt::format("{}/{}/images"))) {
      exec(fmt::format("zfs create -o canmount={} {}/{}/images", pool_name, mount_name));
    }
    if (!exists(fmt::format("{}/{}/volumes"))) {
      exec(fmt::format("zfs create -o canmount={} {}/{}/volumes", pool_name, mount_name));
    }
    if (!exists(fmt::format("{}/{}/jails"))) {
      exec(fmt::format("zfs create -o canmount={} {}/{}/jails", pool_name, mount_name));
    }
  }

  std::string zfs::handler::zfs_name(const std::string &path) {
    auto result = exec(fmt::format("zfs list -o name -H {}", path));
    if (!result.isOk()) {
      spdlog::get("app")->error(result.unwrapErr());
      return "";
    }
    auto name = result.unwrap();
    return name.erase(0, name.find(" "));
  }

  bool handler::exists(const std::string &name) {
    auto result = this->exec(fmt::format("zfs list {}", name));
    return result.isOk();
  }

  Result<std::string, std::string> zfs::handler::exec(const std::string &cmd) {
    std::vector<char> outcome;
    std::vector<char> error;
    std::streamsize n;
    const redi::pstream::pmode mode = redi::pstreams::pstdout | redi::pstream::pstderr;
    redi::ipstream run(cmd, mode);
    while (!run.eof()) {
      if ((n = run.err().readsome(error.data(), 1024) > 0)) {
        run.clear();
      }
      if ((n = run.out().readsome(outcome.data(), 1024) > 0)) {
        run.clear();
      }
    }
    if (error.size() > 0) {
      return Err(std::string(error.begin(), error.end()));
    }
    return Ok(std::string(outcome.begin(), outcome.end()));
  }

  handler::~handler() {}
}; // namespace zfs
