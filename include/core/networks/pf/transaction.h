#ifndef __JPOD_CORE_NETWORKING_PF_IOCTL_TRANSACTION__
#define __JPOD_CORE_NETWORKING_PF_IOCTL_TRANSACTION__
#include <memory>
#include <net/if.h>
#include <net/pfvar.h>
#include <stdlib.h>
#include <sys/ioctl.h>

namespace spdlog {
  class logger;
}

namespace core::networking::pf {
  class RuleSet;
  class Transaction {
  public:
    Transaction(int file_descriptor, int elements);
    ~Transaction();
    void begin();
    void commit();
    void rollback();
    RuleSet ruleset(int index);
    int file_descriptor();

  private:
    bool initialize_transaction(struct pfioc_trans *tx, int elements);
    pfioc_trans::pfioc_trans_e *fetch_result_set_at_index(struct pfioc_trans *tx, int index);
    void free_transaction(struct pfioc_trans *tx);
    pfioc_trans tx;
    int _file_descriptor;
    int elements;
    std::shared_ptr<spdlog::logger> logger;
  };
};     // namespace core::networking::pf
#endif // __JPOD_CORE_NETWORKING_PF_IOCTL_TRANSACTION__