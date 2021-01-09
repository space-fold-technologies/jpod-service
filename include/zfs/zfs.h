#ifndef __JPOD_FILESYSTEM_ZFS__
#define __JPOD_FILESYSTEM_ZFS__

#include <string>

namespace fs {
  const std::string VOLUME_FOLDER = "/volumes";
  const std::string JAILS_FOLDER = "/jails";
  const std::string IMAGES_FOLDER = "/images";

  class ZFSManager {
    ZFSManager(const std::string &root, const std::string &base_folder);
    ~ZFSManager();
    std::string find_snapshot(const std::string &hash);
    std::string create_snapshot(const std::string &hash);

  private:
    void initialize();
    bool create_folders();
    bool create_data_sets();
    bool has_dataset(const std::string &name);
    bool create_dataset(const std::string &name);
    bool create_dataset(const std::string &name, const std::string &mount_point);
    bool has_path(const std::string &path);
    bool create_folder(const std::string &path);
    std::string get_poolname();
    std::string get_name(const std::string &path);
    std::string get_untag();
    const std::string &root;
    const std::string &base_folder;
  };
}; // namespace fs

#endif // __JPOD_FILESYSTEM_ZFS__
