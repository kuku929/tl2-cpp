#include "stm.h"
#include "tl2/tl2.h"
#include <array>
#include <gtest/gtest.h>
#include <thread>
using namespace tl2;
struct PetersonLock {
public:
  PetersonLock() : flag({0, 0}), victim(0) { ; }
  void lock(uint id) {
    atomically([&]() {
      flag[id] = 1u;
      victim = id;
      while (static_cast<uint>(flag[1 - id]) == 1 and static_cast<uint>(victim) == id) {}
    });
  };
  void unlock(uint id) {
    atomically([&]() { flag[id] = 0; });
  };

private:
  std::array<TVar<uint>, 2> flag;
  TVar<uint> victim;
};

uint run(const uint niters) {
  PetersonLock l;
  uint counter = 0;
  std::thread t1([&]() {
    for (uint _ = 0; _ < niters; ++_) {
      l.lock(0);
      counter++;
      l.unlock(0);
    }
  });
  std::thread t2([&]() {
    for (uint _ = 0; _ < niters; ++_) {
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
  for (uint _ = 1; _ < 100'000; _ += 10'000) {
    EXPECT_EQ(run(_), 2 * _);
  }
}
