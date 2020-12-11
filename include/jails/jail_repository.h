#ifndef __JPOD_JAIL_REPOSITORY__
#define __JPOD_JAIL_REPOSITORY__
#include <jails/models.h>
#include <string>
#include <vector>

namespace prison {
  class PrisonRepository {
  public:
    virtual void save(const Details &details) = 0;
    virtual Details fetchByReference(const std::string &reference) = 0;
    virtual Details fetchByNameOrReference(const std::string &value) = 0;
    virtual std::vector<Details> fetchAll() = 0;
    virtual void remove(const std::string &reference) = 0;
  };
};     // namespace prison
#endif // __JPOD_JAIL_REPOSITORY__
