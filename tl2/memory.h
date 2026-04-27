#pragma once
#include "write_set.h"
#include <array>
#include <cstddef>
#include <memory>
#include <memory_resource>

#define TL2_THREAD_STORE_SIZE 4096 // in bytes

namespace tl2::internal {
// Single shared synchronized pool resource for those logs that are created
// with an external resource (e.g., to share allocations across threads).

class PerThreadPolicy {
public:
  template <typename WS> void clear(WS &w) { return res.release(); }

  void *allocate(std::size_t bytes, std::size_t alignment) {
    return res.allocate(bytes, alignment);
  }

private:
  std::pmr::monotonic_buffer_resource res{TL2_THREAD_STORE_SIZE};
};

class SynchronizedPoolPolicy {
public:
  template <typename WS> void clear(WS &w) {
    for (const auto &op : w) {
      res.deallocate(reinterpret_cast<void *>(op.val_addr()), op.bytes_size());
    }
  }

  void *allocate(std::size_t bytes, std::size_t alignment) {
    return res.allocate(bytes, alignment);
  }

private:
  static std::pmr::synchronized_pool_resource res;
};

inline std::pmr::synchronized_pool_resource SynchronizedPoolPolicy::res{};
} // namespace tl2::internal