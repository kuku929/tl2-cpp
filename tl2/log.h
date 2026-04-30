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

  template <typename T>
  std::optional<T *> __attribute__((always_inline))
  value_at(const T *addr) const {
    // check in write set for this address
    std::optional<T *> result(std::nullopt);
    if (const auto entry = w.find_opt(to_addr(addr)); entry.has_value()) {
      result = reinterpret_cast<T *>(entry.value());
    }
    return result;
  }

  template <typename T>
  __attribute__((always_inline)) void append_read(const T *addr) {

    r.update({to_addr(addr), hashtbl[to_addr(addr)].get_version()});
  }

  template <typename T>
  __attribute__((always_inline)) void append_write(const T *addr, T &&val) {
    /*
    val is a universal reference.
    If given as a lvalue we will copy(slow).
    If given as a rvalue we will move(fast).
    */
    if (const auto entry = w.find_opt(to_addr(addr)); entry.has_value()) {
      // Address already in write-set: overwrite existing staged value.
      *reinterpret_cast<T *>(entry.value()) = std::forward<T>(val);
      return;
    }
    // First write for this address in this transaction: stage a copied value.
    T *obj =
        new (store.allocate(sizeof(T), alignof(T))) T(std::forward<T>(val));
    w.update(WriteOp(addr, obj));
  }

  void clear() {
    store.clear(w);
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
// inline static thread_local Log<WriteOrderedSet, ReadOrderedSet,
// SynchronizedPoolPolicy> log;
inline static thread_local Log<WriteHashVectorSet, ReadOrderedSet,
                               PerThreadPolicy>
    log;
// inline static thread_local Log<WriteOrderedSet, ReadOrderedSet,
// PerThreadPolicy> log;
} // namespace tl2::internal