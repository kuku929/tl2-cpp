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
#include <type_traits>
#include <utility>
#include <vector>

#ifdef DEBUG
#include <iostream>
template <>
struct std::formatter<tl2::internal::ReadOp>
    : public std::formatter<
          std::pair<tl2::internal::addr_t, tl2::internal::version_t>> {
  using Parent = std::formatter<
      std::pair<tl2::internal::addr_t, tl2::internal::version_t>>;
  using Parent::parse;
  auto format(const tl2::internal::ReadOp &op,
              std::format_context &format_ctx) const {
    return Parent::format(std::pair(op.addr(), op.version()), format_ctx);
  }
  template <class Context>
  Context::iterator format(const tl2::internal::ReadOp &t, Context &ctx) const;
};

template <>
struct std::formatter<tl2::internal::WriteOp>
    : public std::formatter<std::vector<int>> {
  using Parent = std::formatter<std::vector<int>>;
  using Parent::parse;
  auto format(const tl2::internal::WriteOp &op,
              std::format_context &format_ctx) const {
    return Parent::format(op.value<std::vector<int>>(), format_ctx);
  }
  template <class Context>
  Context::iterator format(const tl2::internal::ReadOp &t, Context &ctx) const;
};
template <typename T>
  requires std::formattable<T, char>
inline void print(T &obj, char end = '\n') {
  std::cerr << std::format("{0}", obj) << end;
}
#endif

namespace tl2::internal {
using namespace tl2::internal;
template <typename WriteSetT, typename ReadSetT, typename StorePolicy>
class Log {
public:
  Log() : r(), w(), store(StorePolicy()) {}

  template <typename T> std::optional<T> 
  __attribute__((always_inline)) value_at(const T *addr) const {
    // check in write set for this address
    const std::optional<addr_t> entry =
        w.find_opt(reinterpret_cast<addr_t>(addr));
    if (entry.has_value()) {
      return *reinterpret_cast<const T *>(entry.value());
    }
    return std::nullopt;
  }

  template <typename T> 
  __attribute__((always_inline)) void append_read(const T *addr) {
    const addr_t a = reinterpret_cast<addr_t>(addr);
    const ReadOp &op = {a, hashtbl[a].get_version()};
    r.update(op);
  }

  template <typename T>
  __attribute__((always_inline)) void append_write(const T *addr, const T &val) noexcept {
    const addr_t a = reinterpret_cast<addr_t>(addr);
    const auto entry = w.find_opt(a);
    if (entry.has_value()) {
      // Address already in write-set: overwrite existing staged value.
      T *val_addr = reinterpret_cast<T *>(entry.value());
      *val_addr = std::move(val);
      return;
    }

    // First write for this address in this transaction: stage a copied value.
    void *storage = store.allocate(sizeof(T), alignof(T));
    T *obj = new (storage) T(std::move(val));
    const auto &op = WriteOp(addr, obj);
    w.update(op);
  }

  void clear() {
    store.clear(w);
    r.clear();
    w.clear();
  }

  void deallocate_resources() {
    store.deallocate(w);
    r.clear();
    w.clear();
  }

  WriteSetT &writes() noexcept { return w; }

  ReadSetT &reads() noexcept { return r; }

private:
  ReadSetT r;
  WriteSetT w;
  StorePolicy store;
};
// inline static thread_local Log<WriteOrderedSet, ReadOrderedSet, SynchronizedPoolPolicy> log;
using DefaultReadSet = ReadOrderedSet;
// using DefaultReadSet = ReadVectorSet;
// using DefaultReadSet = ReadHashVectorSet;
inline static thread_local Log<WriteHashVectorSet, ReadVectorSet, PerThreadPolicy> log;
// inline static thread_local Log<WriteOrderedSet, ReadOrderedSet, PerThreadPolicy> log;
} // namespace tl2::internal