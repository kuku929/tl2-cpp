/*
Test that atomically() restores lvalue arguments on retry.
*/

#include "tl2/tl2.h"
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace tl2;

struct A {
  A() : a(0) { ; }
  A(const A &other) {
    a = other.a;
    std::cout << "copy()" << std::endl;
  }
  A &operator=(const A &) = default;
  int a;
};

TEST(SimpleTests, RestoreArgs) {
  const int iters = 1;
  for (int _ = 0; _ < iters; ++_) {
    std::atomic<bool> start;
    start.store(false, std::memory_order_release);
    bool acquired{false};
    A test;
    TVar<int> lock(0);

    std::thread t1([&]() noexcept {
      atomically([&]() {
        while (!start.load(std::memory_order_acquire)) {
        }
        lock = 1;
      });
    });

    std::thread t2([&]() noexcept {
      atomically(
          [&]() {
            if (static_cast<int>(lock) == 0) {
              start.store(true, std::memory_order_release);
              while (static_cast<int>(lock) == 0) {
              }
              acquired = true;
            }
          },
          acquired, test);
    });

    t1.join();
    t2.join();

    EXPECT_FALSE(acquired);
  }
}