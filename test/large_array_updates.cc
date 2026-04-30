/*
This test ensures no deadlocks occur
while acquiring locks in the write set.

A good performance indicator for the read/write-set
insertion/lookup times.
*/
#include "hash_table.h"
#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace tl2;

TEST(SimpleTests, SingleThreadedLargeArrayAccess) {
  const int N = tl2::internal::LOCKTABLE_SIZE;
  std::vector<TVar<int>> A(N, 0);

  atomically([&]() {
    for (int i = 0; i < N; ++i) {
      int val = static_cast<int>(A[i]);
      A[i] = val + 1;
    }
  });

  atomically([&]() {
    for (int i = 0; i < N; ++i) {
      ASSERT_EQ(static_cast<int>(A[i]), 1);
    }
  });
}