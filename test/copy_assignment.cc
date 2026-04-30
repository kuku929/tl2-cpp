/*
Tests if copy assignment of TVar works without errors.
*/
#include "hash_table.h"
#include "stm.h"
#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

using namespace tl2;

TEST(SimpleTests, CopyAssignmentSingle) {
  TVar<int> a(0), b(1);
  atomically([&]() { a = b; });
  int val_a;
  {
    atomically([&]() { val_a = static_cast<int>(a); });
  }
  int val_b;
  {
    atomically([&]() { val_b = static_cast<int>(b); });
  }
  ASSERT_EQ(val_a, 1);
  ASSERT_EQ(val_b, 1);
  atomically([&]() {
    // there is no difference
    b = 2;
    a = std::move(b);
  });
  {
    atomically([&]() { val_a = static_cast<int>(a); });
  }
  {
    atomically([&]() { val_b = static_cast<int>(b); });
  }
  ASSERT_EQ(val_a, 2);
  ASSERT_EQ(val_b, 2);
}

TEST(SimpleTests, CopyAssignmentDouble) {
  const int N = 1000;
  std::vector<TVar<int>> a(N, 0);
  std::vector<TVar<int>> b(N, 1);
  std::thread t1([&]() {
    for (int i = 0; i < N; ++i) {
      atomically([&]() { a[i] = b[i]; });
    }
  });
  std::thread t2([&]() {
    for (int i = 0; i < N; ++i) {
      atomically([&]() { a[i] = b[i]; });
    }
  });
  t1.join();
  t2.join();
  atomically([&]() {
    for (int i = 0; i < N; ++i) {
      ASSERT_EQ(static_cast<int>(a[i]), 1);
      ASSERT_EQ(static_cast<int>(b[i]), 1);
    }
  });
}