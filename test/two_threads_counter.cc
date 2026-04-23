#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
using namespace tl2;

uint run(const uint niters) {
  TVar<uint> counter(0);
  std::thread t1([&]() {
    atomically([&]() {
      for (uint _ = 0; _ < niters; ++_) {
        uint val = static_cast<uint>(counter);
        counter = val + 1;
      }
    });
  });
  std::thread t2([&]() {
    atomically([&]() {
      for (uint _ = 0; _ < niters; ++_) {
        uint val = static_cast<uint>(counter);
        counter = val + 1;
      }
    });
  });
  t1.join();
  t2.join();
  uint ans = 0;
  {
    atomically([&]() { ans = static_cast<uint>(counter); });
  }
  return ans;
}

TEST(SimpleTests, TwoThreadsCounter) {
  for (uint _ = 1; _ < 100'000; _ += 10'000) {
    EXPECT_EQ(run(_), 2 * _);
  }
}
