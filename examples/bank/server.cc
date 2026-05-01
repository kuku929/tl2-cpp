#include "server.h"
#include "examples/bank/transaction.h"
#include "stm.h"
#include <algorithm>
#include <chrono>
#include <memory>
#include <random>
#include <thread>
#include <vector>

const int MAX_THREADS = 8;

void Server::spin() {
  // in one iteration we spawn some
  // number of threads and await.
  int num_transactions = random_int(2 * MAX_THREADS);
  std::cout << std::format("using {0} threads.", num_transactions) << std::endl;
  for (int _ = 0; _ < num_transactions; ++_) {
    auto transaction = [&]() {
      switch (random_int(1)) {
      case 0:
        return std::function<void(void)>([&]() {
          Add t = construct_add();
          logger.log_add(t.name(), t.amt());
          t.execute();
        });
      case 1:
        return std::function<void(void)>([&]() -> void {
          Withdraw t = construct_withdraw();
          logger.log_withdraw(t.name(), t.amt());
          t.execute();
        });
      }
      return std::function<void(void)>([&]() {
        Transfer t = construct_transfer();
        logger.log_transfer(t.src(), t.dest(), t.amt());
        t.execute();
      });
    }();
    if (running.size() == MAX_THREADS) {
      std::for_each(running.begin(), running.end(),
                    [](std::thread &t) { t.join(); });
      running.clear();
    }
    running.emplace_back(std::thread(transaction));
    std::this_thread::sleep_for(std::chrono::milliseconds(random_int(10)));
  }
  std::for_each(running.begin(), running.end(),
                [](std::thread &t) { t.join(); });
  running.clear();
}

std::string Server::random_name() {
  static std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<size_t> dist(0, names.size() - 1);
  return names[dist(rng)];
}

int Server::random_int(int max, int min) {
  static std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<size_t> dist(min, max);
  return dist(rng);
}

Withdraw Server::construct_withdraw() { return Withdraw(random_name(), random_int(1000)); }

Transfer Server::construct_transfer() { return Transfer(random_name(), random_name(), random_int(1000)); }

Add Server::construct_add() { return Add(random_name(), random_int(1000)); }