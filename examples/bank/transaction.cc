#include "transaction.h"
#include "stm.h"
#include "tl2/tl2.h"

using namespace tl2;

void Withdraw::execute() const {
  atomically([]() {

  }) return;
}