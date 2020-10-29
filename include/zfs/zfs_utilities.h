#ifndef __ZFS_UTILITIES__
#define __ZFS_UTILITIES__

#include <result.h>

namespace zfs {
  class handler {
  public:
    handler(const std::string &pool_name, const std::string &mount_point, const std::string &mount_name);
    std::string zfs_name(const std::string &path);
    void startup();
    ~handler();

  private:
    bool exists(const std::string &name);
    const std::string &pool_name;
    const std::string &mount_point;
    const std::string &mount_name;
    Result<std::string, std::string> exec(const std::string &cmd);
  };
};     // namespace zfs
#endif // __ZFS_UTILITIES__
