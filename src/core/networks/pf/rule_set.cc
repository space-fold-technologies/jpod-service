#include <core/networks/pf/rule_set.h>
#include <core/networks/pf/rules.h>
#include <core/networks/pf/transaction.h>
#include <spdlog/spdlog.h>
#include <sys/ioctl.h>

namespace core::networking::pf {
RuleSet::RuleSet(Transaction *transaction, pfioc_trans::pfioc_trans_e *element) noexcept
  : transaction(transaction),
    element(element), logger(spdlog::get("jpod")) {}
void RuleSet::add_rule(const Rule &rule)
{
  auto content = rule.content();
  if (ioctl(transaction->file_descriptor(), DIOCADDRULE, &content) == -1) {
    logger->error("FAILED TO ADD RULE");
  }
}
void RuleSet::set_anchor(const std::string &name)
{
  std::memcpy(this->element->anchor, name.c_str(), name.size());
}
std::string RuleSet::anchor()
{
  return std::string(this->element->anchor);
}
RuleSetType RuleSet::type()
{
  return static_cast<RuleSetType>(this->element->rs_num);
}
RuleSet::~RuleSet() {}
}// namespace core::networking::pf