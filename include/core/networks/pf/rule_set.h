#ifndef __JPOD_SERVICE_CORE_NETWORKS_PF_ROUTING_RULESET__
#define __JPOD_SERVICE_CORE_NETWORKS_PF_ROUTING_RULESET__

#include <net/if.h>
#include <net/pfvar.h>
#include <string>
#include <memory>

namespace spdlog {
  class logger;
}

namespace core::networks::pf {

  enum class RuleSetType {
    SCRUB = PF_RULESET_SCRUB,
    FILTER = PF_RULESET_FILTER,
    NAT = PF_RULESET_NAT,
    BI_NAT = PF_RULESET_BINAT,
    RE_DIRECT = PF_RULESET_RDR,
    ALTQ = PF_RULESET_ALTQ,
    TABLE = PF_RULESET_TABLE
  };

  class Transaction;
  class Rule;
  class RuleSet final {

  public:
    RuleSet(Transaction *transaction, pfioc_trans::pfioc_trans_e *element) noexcept;
    ~RuleSet();
    void add_rule(const Rule &rule);
    void set_anchor(const std::string &name);
    std::string anchor();
    RuleSetType type();

  private:
    Transaction *transaction;
    pfioc_trans::pfioc_trans_e *element;
    std::shared_ptr<spdlog::logger> logger;
  };
} // namespace core::networks::pf
#endif // __JPOD_SERVICE_CORE_NETWORKS_PF_ROUTING_RULESET__