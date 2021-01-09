#ifndef __JPOD_FILESYSTEM_TYPES__
#define __JPOD_FILESYSTEM_TYPES__

#include <string>

namespace fs {
  struct DataSet {
    std::string name;
    std::string type;
    bool mounted;
    std::string mount_point;
    std::string origin;
  };
} // namespace fs

#endif // __JPOD_FILESYSTEM_TYPES__