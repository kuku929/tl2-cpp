/*
tl2-cpp supports nested transactions.
This code tests the same.
*/
#include <gtest/gtest.h>
#include <thread>
#include <stdexcept>
#include "tl2/tl2.h"

using namespace tl2;

TEST(NestedTransactionTest, BasicNesting) {
  TVar<int> x(0);

  atomically([&]() {
    atomically([&]() {
      x = 10;
    });

    int val = static_cast<int>(x);
    EXPECT_EQ(val, 10);
  });

  int final = atomically([&]() {
    return static_cast<int>(x);
  });

  EXPECT_EQ(final, 10);
}

TEST(NestedTransactionTest, ReadModifyWrite) {
  TVar<int> x(1);

  atomically([&]() {
    int outer_read = static_cast<int>(x);

    atomically([&]() {
      x = outer_read + 1;
    });

    EXPECT_EQ(static_cast<int>(x), 2);
  });

  int final = atomically([&]() { return static_cast<int>(x); });
  EXPECT_EQ(final, 2);
}

TEST(NestedTransactionTest, InnerAbortPropagates) {
  TVar<int> x(0);

  try {
    atomically([&]() {
      x = 1;

      atomically([&]() {
        throw std::runtime_error("abort");
      });

      // Should never execute if abort propagates correctly
      x = 2;
    });
  } catch (...) {
  }

  int final = atomically([&]() { return static_cast<int>(x); });

  EXPECT_EQ(final, 0);
}

TEST(NestedTransactionTest, ConcurrentNested) {
  TVar<int> x(0);

  auto worker = [&]() {
    for (int i = 0; i < 1000; ++i) {
      atomically([&]() {
        atomically([&]() {
          x = static_cast<int>(x) + 1;
        });
      });
    }
  };

  std::thread t1(worker);
  std::thread t2(worker);

  t1.join();
  t2.join();

  int final = atomically([&]() { return static_cast<int>(x); });

  EXPECT_EQ(final, 2000);
}