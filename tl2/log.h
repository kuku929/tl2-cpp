#pragma once
#include "hash_table.h"
#include "memory.h"
#include "read_set.h"
#include "types.h"
#include "write_set.h"
#include <cassert>
#include <cstring>
#include <memory>
#include <memory_resource>
#include <optional>

namespace tl2::internal {
using namespace tl2::internal;
template <typename WriteSetT, typename ReadSetT, typename StorePolicy>
class Log {
public:
  // Default: use per-log monotonic resource backed by default upstream
  // allocation
  Log() : r(), w(), store(StorePolicy()) {}

  template <typename T> std::optional<T> value_at(const T *addr) const {
    // check in write set for this address
    const std::optional<addr_t> entry =
        w.find_opt(reinterpret_cast<addr_t>(addr));
    if (entry.has_value()) {
      return *reinterpret_cast<const T *>(entry.value());
    }
    return std::nullopt;
  }

  template <typename T> void append_read(const T *addr) {
    const addr_t a = reinterpret_cast<addr_t>(addr);
    const ReadOp &op = {a, hashtbl[a].get_version()};
    r.update(op);
  }

  template <typename T>
  void append_write(const T *addr, const T &val) noexcept {
    const addr_t a = reinterpret_cast<addr_t>(addr);
    const auto entry = w.find_opt(a);
    if (entry.has_value()) {
      // Address already in write-set: overwrite existing staged value.
      std::memcpy(reinterpret_cast<void *>(entry.value()),
                  reinterpret_cast<const void *>(&val), sizeof(T));
      return;
      // w.modify not required
    }

    // First write for this address in this transaction: stage a copied value.
    const std::size_t nbytes = sizeof(T);
    void *storage = store.allocate(nbytes, alignof(T));
    std::memcpy(storage, reinterpret_cast<const void *>(&val), nbytes);
    const T *copied_ptr = reinterpret_cast<const T *>(storage);
    const WriteOp &op = {addr, copied_ptr};
    w.update(op);
  }

  void clear() {
    store.clear(w);
    r.clear();
    w.clear();
  }

  WriteSetT &writes() { return w; }

  ReadSetT &reads() { return r; }

private:
  ReadSetT r;
  WriteSetT w;
  StorePolicy store;
};
inline static thread_local Log<WriteOrderedSet, ReadOrderedSet, SynchronizedPoolPolicy>
    log;
// inline static thread_local Log<WriteOrderedSet, ReadOrderedSet>
// log(&internal::synchronized_pool_resource);
} // namespace tl2::internal