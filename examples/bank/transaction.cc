#include "transaction.h"
#include <optional>
#include "account.h"
#include "stm.h"
#include "tl2/tl2.h"

using namespace tl2;

void Withdraw::execute() const {
  tl2::atomically([&]() {
    Account upd{.balance = m_amt, .name = m_name};
    if(std::optional<Account> a = accounts.get(m_name); a.has_value()) {
      upd = remove_balance(a.value(), m_amt);
    }
    accounts.set(m_name, upd);
    return;
  });
}

void Add::execute() const {
  tl2::atomically([&]() {
    Account upd{.balance = m_amt, .name = m_name};
    if(std::optional<Account> a = accounts.get(m_name); a.has_value()) {
      upd = add_balance(a.value(), m_amt);
    }
    accounts.set(m_name, upd);
    return;
  });
}

void Transfer::execute() const {
  // A great example of composibility
  // of transactions.
  tl2::atomically([&] {
    Add(m_dest, m_amt).execute();
    Withdraw(m_src, m_amt).execute();
  });
}
