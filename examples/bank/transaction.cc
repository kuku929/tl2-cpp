#include "transaction.h"
#include "account.h"
#include "stm.h"
#include "tl2/tl2.h"

using namespace tl2;

void Withdraw::execute() const {
  Account a = accounts.get(m_name);
  accounts.set(m_name, remove_balance(a, m_amt));
  return;
}

void Add::execute() const {
  Account a = accounts.get(m_name);
  accounts.set(m_name, remove_balance(a, m_amt));
  return;
}

void Transfer::execute() const {
  // A great example of composibility
  // of transactions.
  tl2::atomically([&]{
    Add(m_dest, m_amt).execute();
    Withdraw(m_src, m_amt).execute();
  });
}
