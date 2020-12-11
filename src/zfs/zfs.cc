#include <algorithm>
#include <boost/filesystem.hpp>
#include <pstream.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/spdlog.h>
#include <utilities/commander.h>
#include <zfs/zfs.h>

namespace fs {
  ZFSManager::ZFSManager(const std::string &root, const std::string &base_folder) : root(root), base_folder(base_folder) {
  }

  void ZFSManager::initialize() {
  }

  std::string fs::ZFSManager::find_snapshot(const std::string &hash) {
    return "expression";
  }

  std::string fs::ZFSManager::create_snapshot(const std::string &hash) {
    return "";
  }

  bool ZFSManager::create_folders() {
    bool failed_operation = false;
    if (!has_path(base_folder)) {
      if (!create_folder(base_folder)) {
        spdlog::get("app")->error("FAILED TO CREATE {}", base_folder);
        failed_operation = true;
      }
    }
    if (!has_path(fmt::format("{}/{}", base_folder, JAILS_FOLDER))) {
      if (!create_folder(fmt::format("{}/{}", base_folder, JAILS_FOLDER))) {
        spdlog::get("app")->error("FAILED TO CREATE {}", fmt::format("{}/{}", base_folder, JAILS_FOLDER));
        failed_operation = true;
      }
    }
    if (!has_path(fmt::format("{}/{}", base_folder, IMAGES_FOLDER))) {
      if (!create_folder(fmt::format("{}/{}", base_folder, IMAGES_FOLDER))) {
        spdlog::get("app")->error("FAILED TO CREATE {}", fmt::format("{}/{}", base_folder, IMAGES_FOLDER));
        failed_operation = true;
      }
    }
    if (!failed_operation) {
      std::filesystem::permissions(base_folder, std::filesystem::perms::owner_all | std::filesystem::perms::group_all,
                                   std::filesystem::perm_options::add);
    }
    return failed_operation;
  }

  bool ZFSManager::create_data_sets() {
    auto pool_name = get_poolname();
    if (!has_dataset(fmt::format("{}/jpod", pool_name))) {
      if (!create_dataset(fmt::format("{}/jpod", pool_name), base_folder)) {
        return false;
      }
    }
    if (!has_dataset(fmt::format("{}/jpod/images", pool_name))) {
      if (!create_dataset(fmt::format("{}/jpod/images", pool_name))) {
        return false;
      }
    }
    if (!has_dataset(fmt::format("{}/jpod/images", pool_name))) {
      if (!create_dataset(fmt::format("{}/jpod/images", pool_name))) {
        return false;
      }
    }
    if (!has_dataset(fmt::format("{}/jpod/volumes", pool_name))) {
      if (!create_dataset(fmt::format("{}/jpod/volumes", pool_name))) {
        return false;
      }
    }
    if (!has_dataset(fmt::format("{}/jpod/jails", pool_name))) {
      if (!create_dataset(fmt::format("{}/jpod/jails", pool_name))) {
        return false;
      }
    }
    return true;
  }

  bool ZFSManager::has_dataset(const std::string &name) {
    const auto order = fmt::format("zfs list {}", name);
    auto result = utilties::Runner::execute(order);
    if (result.isErr()) {
      spdlog::get("app")->error("{}", result.unwrapErr());
    }
    return result.isOk();
  }

  bool ZFSManager::create_dataset(const std::string &name) {
    const auto order = fmt::format("zfs create -o canmount=off -o {}", name);
    auto result = utilties::Runner::execute(order);
    if (result.isErr()) {
      spdlog::get("app")->error("FAILED TO CREATE DATASET: {}", result.unwrapErr());
    }
    return result.isOk();
  }

  bool ZFSManager::create_dataset(const std::string &name, const std::string &mount_point) {
    const auto order = fmt::format("zfs create -o canmount=off -o mountpoint={} {}", mount_point, name);
    auto result = utilties::Runner::execute(order);
    if (result.isErr()) {
      spdlog::get("app")->error("FAILED TO CREATE DATASET: {}", result.unwrapErr());
    }
    return result.isOk();
  }

  bool ZFSManager::has_path(const std::string &path) {
    return std::filesystem::directory_entry(path).exists();
  }

  bool ZFSManager::create_folder(const std::string &path) {
    return std::filesystem::create_directories(path);
  }

  std::string ZFSManager::get_poolname() {
    const std::string command = "zfs list -o name -H /";
    auto result = utilties::Runner::execute(command);
    if (result.isErr()) {
      spdlog::get("app")->error("ERROR:{}", result.unwrapErr());
      return "";
    }
    const std::string output = result.unwrap();
    if (auto result = output.find_first_of("/"); result != std::string::npos) {
      return output.substr(0, result);
    }
    return "";
  }

  std::string ZFSManager::get_name(const std::string &path) {
    const std::string command = fmt::format("zfs list -o name -H {}", path);
    auto result = utilties::Runner::execute(command);
    if (result.isErr()) {
      spdlog::get("app")->error("ERROR:{}", result.unwrapErr());
      return "";
    }
    const std::string output = result.unwrap();
    if (auto result = output.find_first_of("/"); result != std::string::npos) {
      return output.substr(0, result);
    }
    return "";
  }

  std::string ZFSManager::get_untag() {
    return "";
  }

  ZFSManager::~ZFSManager() {
  }
} // namespace fs
