#include <core/networking/pf/rule_set.h>
#include <core/networking/pf/transaction.h>
#include <definitions.h>
#include <spdlog/spdlog.h>
#include <stdio.h>

namespace networking::pf {
  Transaction::Transaction(int file_descriptor, int elements)
      : _file_descriptor(file_descriptor), elements(elements) {
    initialize_transaction(&this->tx, elements);
  }
  void Transaction::begin() {
    if (ioctl(_file_descriptor, DIOCXBEGIN, &tx) == -1) {
      spdlog::get(LOGGER)->error("FAILED TO BEGIN TRANSACTION (/dev/pf)");
    }
  }

  void Transaction::commit() {
    if (ioctl(_file_descriptor, DIOCXCOMMIT, &tx) == -1) {
      spdlog::get(LOGGER)->error("FAILED TO COMMIT TRANSACTION (/dev/pf)");
    }
  }

  void Transaction::rollback() {
    if (ioctl(_file_descriptor, DIOCXROLLBACK, &tx) == -1) {
      spdlog::get(LOGGER)->error("FAILED TO COMMIT TRANSACTION (/dev/pf)");
    }
  }

  RuleSet Transaction::ruleset(int index) {
    return RuleSet(this, fetch_result_set_at_index(&tx, index));
  }

  bool Transaction::initialize_transaction(struct pfioc_trans *tx, int elements) {
    tx->size = elements;
    tx->esize = elements * sizeof(struct pfioc_trans::pfioc_trans_e);
    tx->array = (struct pfioc_trans::pfioc_trans_e *)calloc(elements, sizeof(struct pfioc_trans::pfioc_trans_e));
    if (tx->array == NULL) {
      return 1;
    }
    return true;
  }

  int Transaction::file_descriptor() {
    return _file_descriptor;
  }

  pfioc_trans::pfioc_trans_e *Transaction::fetch_result_set_at_index(struct pfioc_trans *tx, int index) {
    if (index >= int(sizeof(tx->array)) || index < 0) {
      //ILLEGAL OPERATION
    }
    return &tx->array[index];
  }

  void Transaction::free_transaction(struct pfioc_trans *tx) {
    if (tx->array != NULL) {
      free(tx->array);
    }
    tx->size = 0;
    tx->esize = 0;
  }

  Transaction::~Transaction() {
  }
} // namespace networking::pf