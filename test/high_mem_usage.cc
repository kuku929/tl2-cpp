/*
During a write TL2 will save the value to be written in a buffer.
This test creates a big TVar to check for issues in this mechanism.
If this test fails, look at tl2/memory.h and tl2/log.h
*/
#include "hash_table.h"
#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <iostream>
#include <thread>
#include <vector>
using namespace tl2;

const size_t LARGE_SIZE = 100'000;

int run_four(const uint niters) {
  const auto size = LARGE_SIZE / 4;
  TVar<std::vector<int>> tvar(std::vector<int>(size, 0));
  std::thread t1([&]() {
    for (uint i = 0; i < niters; ++i) {
      atomically([&]() {
        std::vector<int> v = static_cast<std::vector<int>>(tvar);
        v[static_cast<size_t>(i % size)] += 1;
        tvar = v;
      });
    }
  });
  std::thread t2([&]() {
    for (uint i = 0; i < niters; ++i) {
      atomically([&]() {
        std::vector<int> v = static_cast<std::vector<int>>(tvar);
        v[i % size] += 1;
        tvar = v;
      });
    }
  });
  std::thread t3([&]() {
    for (uint i = 0; i < niters; ++i) {
      atomically([&]() {
        std::vector<int> v = static_cast<std::vector<int>>(tvar);
        v[i % size] += 1;
        tvar = v;
      });
    }
  });
  std::thread t4([&]() {
    for (uint i = 0; i < niters; ++i) {
      atomically([&]() {
        std::vector<int> v = static_cast<std::vector<int>>(tvar);
        v[i % size] += 1;
        tvar = v;
      });
    }
  });
  t1.join();
  t2.join();
  t3.join();
  t4.join();
  int total = 0;
  {
    atomically([&]() {
      std::vector<int> v = static_cast<std::vector<int>>(tvar);
      for (size_t j = 0; j < size; ++j) {
        total += v[j];
      }
    });
  }
  return total;
}

int run_two(const uint niters) {
  const auto size = LARGE_SIZE / 2;
  TVar<std::vector<int>> tvar(std::vector<int>(size, 0));
  std::thread t1([&]() {
    for (uint i = 0; i < niters; ++i) {
      atomically([&]() {
        std::vector<int> v = static_cast<std::vector<int>>(tvar);
        v[i % size] += 1;
        tvar = v;
      });
    }
  });
  std::thread t2([&]() {
    for (uint i = 0; i < niters; ++i) {
      atomically([&]() {
        std::vector<int> v = static_cast<std::vector<int>>(tvar);
        v[i % size] += 1;
        tvar = v;
      });
    }
  });
  t1.join();
  t2.join();
  int total = 0;
  {
    atomically([&]() {
      std::vector<int> v = static_cast<std::vector<int>>(tvar);
      for (size_t j = 0; j < size; ++j) {
        total += v[j];
      }
    });
  }
  return total;
}

int run_single(const uint niters) {
  TVar<std::vector<int>> tvar(std::vector<int>(LARGE_SIZE, 0));
  for (uint i = 0; i < niters; ++i) {
    atomically([&]() {
      std::vector<int> v = static_cast<std::vector<int>>(tvar);
      v[static_cast<size_t>(i % LARGE_SIZE)] += 1;
      tvar = v;
    });
  }
  int total = 0;
  {
    atomically([&]() {
      std::vector<int> v = static_cast<std::vector<int>>(tvar);
      for (size_t j = 0; j < LARGE_SIZE; ++j) {
        total += v[j];
      }
    });
  }
  return total;
}

TEST(SimpleTests, HighMemUsageSingle) {
  constexpr int iters = 100;
  EXPECT_EQ(run_single(iters), iters);
}

TEST(SimpleTests, HighMemUsageDouble) {
  constexpr int iters = 100;
  EXPECT_EQ(run_two(iters), 2 * iters);
}

TEST(SimpleTests, HighMemUsageQuadruple) {
  constexpr int iters = 100;
  EXPECT_EQ(run_four(iters), 4 * iters);
}