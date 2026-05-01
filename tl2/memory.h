#pragma once
#include "op.h"
#include <array>
#include <cstddef>
#include <memory>
#include <memory_resource>

#define TL2_THREAD_STORE_SIZE 4096 // in bytes

namespace tl2::internal {
class PerThreadPolicy {
public:
  template <typename LS> void clear(LS &l) {
    for (auto &loc : l) {
      if (loc.was_written()) {
        loc.write_val.free_heap();
      }
    }
    return res.release();
  }

  void *allocate(std::size_t bytes, std::size_t alignment) {
    return res.allocate(bytes, alignment);
  }

private:
  std::pmr::monotonic_buffer_resource res{TL2_THREAD_STORE_SIZE};
};

class SynchronizedPoolPolicy {
public:
  template <typename LS> void clear(LS &l) {
    for (auto &loc : l) {
      if (loc.was_written()) {
        WriteVal &wv = loc.write_val;
        wv.free_heap();
        res.deallocate(reinterpret_cast<void *>(wv.val_addr()),
                       wv.bytes_size());
      }
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