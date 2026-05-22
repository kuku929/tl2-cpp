#include "zoo/hash_map.h"
#include <string>

struct Account {
  int balance;
  std::string name;
  void print() {
    // std::cout << std::format("{0}", std::pair(balance, name)) << std::endl;
  }
};

inline Account remove_balance(Account &a, int amt) {
  return Account{.balance = a.balance - amt, .name = a.name};
}

inline Account add_balance(Account &a, int amt) {
  return remove_balance(a, -amt);
}

static inline zoo::HashMap<std::string, Account, std::hash<std::string>>
    accounts{std::hash<std::string>()};
