#include <string>
#include "zoo/hash_map.h"

struct Account {
    int balance;
    std::string name;
};

inline Account remove_balance(Account &a, int amt) {
  return Account{
    .balance = a.balance - amt,
    .name = a.name
  };
}

inline Account add_balance(Account &a, int amt) {
    return remove_balance(a, -amt);
}

static inline zoo::HashMap<std::string, Account, std::hash<std::string>> accounts{std::hash<std::string>()};
