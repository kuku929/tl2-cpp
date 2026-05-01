#pragma once
#include "hash_table.h"
#include "memory.h"
#include "op.h"
#include "types.h"
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
template <typename LocationSetT, typename StorePolicy> class Log {
public:
  Log() : l(), store(StorePolicy()) {}

  template <typename T>
  std::optional<T *> __attribute__((always_inline))
  value_at(const T *addr) const {
    // check in the location set for this address
    std::optional<T *> result(std::nullopt);
    if (const auto entry = l.find_write(to_addr(addr)); entry.has_value()) {
      result = reinterpret_cast<T *>(entry.value());
    }
    return result;
  }

  template <typename T>
  __attribute__((always_inline)) void append_read(const T *addr) {
    l.register_read(to_addr(addr));
  }

  template <typename T>
  __attribute__((always_inline)) void append_write(const T *addr, T &&val) {
    /*
    val is a universal reference.
    If given as a lvalue we will copy(slow).
    If given as a rvalue we will move(fast).
    */
    l.register_write(to_addr(addr), std::forward<T>(val), store);
  }

  void clear() {
    store.clear(l);
    l.clear();
  }

  LocationSetT &locations() { return l; }

private:
  LocationSetT l;
  StorePolicy store;
};
} // namespace tl2::internal