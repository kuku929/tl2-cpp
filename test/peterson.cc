#include "tl2/tl2.h"
#include <array>
#include <gtest/gtest.h>
#include <thread>
using namespace tl2;
struct PetersonLock {
public:
  PetersonLock() : flag({0, 0}), victim(0) { ; }
  void lock(int id) {
    atomically([&]() {
      flag[id] = 1;
      victim = id;
      while (static_cast<int>(flag[1 - id]) == 1 and
             static_cast<int>(victim) == id) {
      }
    });
  };
  void unlock(int id) {
    atomically([&]() { flag[id] = 0; });
  };

private:
  std::array<TVar<int>, 2> flag;
  TVar<int> victim;
};

int run(const int niters) {
  PetersonLock l;
  int counter = 0;
  std::thread t1([&]() {
    for (int _ = 0; _ < niters; ++_) {
      l.lock(0);
      counter++;
      l.unlock(0);
    }
  });
  std::thread t2([&]() {
    for (int _ = 0; _ < niters; ++_) {
      l.lock(1);
      counter++;
      l.unlock(1);
    }
  });
  t1.join();
  t2.join();
  return counter;
}

TEST(SimpleTests, PetersonLock) {
  for (int i = 1; i < 100'000; i += 10'000) {
    EXPECT_EQ(run(i), 2 * i);
  }
}
