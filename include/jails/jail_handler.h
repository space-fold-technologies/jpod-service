#ifndef __JPOD_JAIL_HANDLER__
#define __JPOD_JAIL_HANDLER__

#include <functional>
#include <jail.h>
#include <mutex>
#include <result.h>
#include <string>
#include <vector>

namespace prison {
  struct Configuration;
  class PrisonRepository;
  struct Summary {
    int id;
    std::string reference;
    std::string name;
  };

  class PrisonHandler {
    const std::string TEMPORARY_DIRECTOR = "/tmp/jpod.XXXXXXXXXXXXXXXXXXX";

  public:
    PrisonHandler(PrisonRepository &prisonRepository);
    ~PrisonHandler();
    Result<Summary, std::string> create(const Configuration &configuration);
    bool execute(const std::string &script);
    void remove(int id);

  private:
    bool add_parameter(std::vector<jailparam> &parameters, const std::string &key, const std::string &value);
    void save_details(int jid, const std::string reference, const Configuration &configuration);
    std::string format_addresses(const std::vector<std::string> &addresses) const;
    PrisonRepository &prisonRepository;
    std::mutex jail_mutex;
  };
}; // namespace prison

#endif // __JPOD_JAIL_HANDLER__
