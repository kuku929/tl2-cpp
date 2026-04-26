#pragma once
#include "types.h"
#include "version_lock.h"
#include <cstddef>
#include <new>
#include <vector>

namespace tl2::internal {
using namespace tl2::internal;
constexpr size_t LOCKTABLE_SIZE = 1 << 20;

template <std::size_t N>
struct Pad { char data[N]; };
template <>
struct Pad<0> {};

#if defined(__GCC_DESTRUCTIVE_SIZE)
static constexpr std::size_t CACHE_LINE_SIZE = __GCC_DESTRUCTIVE_SIZE;
#elif defined(__cpp_lib_hardware_interference_size)
static constexpr std::size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#else
static constexpr std::size_t CACHE_LINE_SIZE = 64;
#endif

class LockTable {
private:
  static constexpr std::size_t CACHE_LINE = CACHE_LINE_SIZE;

  static constexpr std::size_t lock_size = sizeof(VersionLock);
  static constexpr std::size_t pad_size = (CACHE_LINE - (lock_size % CACHE_LINE)) % CACHE_LINE;

  struct alignas(CACHE_LINE) PaddedVersionLock {
    VersionLock lock;
    Pad<pad_size> pad;
  };
  
  std::vector<PaddedVersionLock> table;
  size_t mask;

public:
  explicit LockTable(const size_t size_pow2)
      : table(size_pow2), mask(size_pow2 - 1) {}

  // maps address to version lock (hashing)
  inline VersionLock &operator[](const addr_t addr) {
    const auto x = reinterpret_cast<uintptr_t>(addr);
    size_t index = (x >> 2) & mask;
    return table[index].lock;
  }
} static hashtbl(LOCKTABLE_SIZE);
} // namespace tl2::internal
