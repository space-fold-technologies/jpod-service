#ifndef __JPOD_NETWORKING_PF_IOCTL_TRANSACTION__
#define __JPOD_NETWORKING_PF_IOCTL_TRANSACTION__
#include <memory>
#include <net/if.h>
#include <net/pfvar.h>
#include <stdlib.h>
#include <sys/ioctl.h>

namespace networking::routing {
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
  };
};     // namespace networking::routing
#endif // __JPOD_NETWORKING_PF_IOCTL_TRANSACTION__