#include "hash_table.h"
#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace tl2;

int main() {
  const int N = tl2::internal::LOCKTABLE_SIZE;
  std::vector<TVar<int>> A(N, 0);

  auto start = std::chrono::high_resolution_clock::now();
  atomically([&]() {
    for (int i = 0; i < N; ++i) {
      int val = static_cast<int>(A[i]);
      A[i] = val + 1;
    }
  });
  auto end = std::chrono::high_resolution_clock::now();
  double seconds = std::chrono::duration<double>(end - start).count();
  std::cerr << seconds << std::endl;
}