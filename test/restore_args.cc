/*
Test that atomically restores lvalue arguments on retry.
*/

#include "tl2/tl2.h"
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace tl2;

TEST(SimpleTests, RestoreArgs) {
  TVar<int> x(1);
  const int iters = 20000;
  int violations=0;
  std::atomic<bool> start{false};

  std::thread writer([&]() {
    while (!start.load(std::memory_order_acquire)) {}

    for (int i = 0; i < iters; ++i) {
      atomically([&]() { x = (i & 1); });
    }
  });

  std::thread reader([&]() {
    while (!start.load(std::memory_order_acquire)) {}

    for (int i = 0; i < iters; ++i) {
      bool flag = false;
      int seen = -1;
      atomically(
          [&](bool& f, int& out) {
            int v = static_cast<int>(x);
            out = v;
            if (v == 1) {
              f = true;
            }
            volatile int spin = 0;
            for (int k = 0; k < 500; ++k) {
              spin += k;
            }
            (void)spin;
          },
          flag, seen);

      if ((seen == 1 && !flag) || (seen == 0 && flag)) {
        violations++;
      }
    }
  });

  start.store(true, std::memory_order_release);
  writer.join();
  reader.join();

  EXPECT_EQ(violations, 0);
}