#pragma once
#include "types.h"
#include "version_lock.h"
#include <vector>

namespace tl2::internal {
using namespace tl2::internal;
constexpr size_t LOCKTABLE_SIZE = 1 << 20;

class LockTable {
private:
  std::vector<VersionLock> table;
  size_t mask;

public:
  explicit LockTable(const size_t size_pow2)
      : table(size_pow2), mask(size_pow2 - 1) {}

  // maps address to version lock (hashing)
  inline VersionLock &operator[](const addr_t addr) {
    const auto x = reinterpret_cast<uintptr_t>(addr);
    size_t index = (x >> 2) & mask;
    return table[index];
  }
} static hashtbl(LOCKTABLE_SIZE);
} // namespace tl2::internal
