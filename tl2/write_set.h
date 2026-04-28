#pragma once
#include "types.h"
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <set>

namespace tl2::internal {
using namespace tl2::internal;
using value_addr_t = addr_t;
class WriteSet;

class WriteOp {
public:
  WriteOp(addr_t a)
      : m_a(std::move(a)), m_val(0), m_destructor([](addr_t obj) noexcept {}),
        m_move([](addr_t src, addr_t dest) noexcept {}), m_sz(0u) {
    ;
  }
  template <typename T>
  [[nodiscard]]
  WriteOp(const T *a, const T *v)
      : m_a(reinterpret_cast<addr_t>(a)), m_val(reinterpret_cast<addr_t>(v)),
        m_destructor([](addr_t obj) noexcept -> void {
          reinterpret_cast<T *>(obj)->~T();
          return;
        }),
        m_move([](addr_t src, addr_t dest) noexcept -> void {
          *reinterpret_cast<T *>(dest) = std::move(*reinterpret_cast<T *>(src));
        }),
        m_sz(sizeof(T)) {
    ;
  }
  addr_t addr() const { return m_a; }

  template <typename T> [[nodiscard]] const T value() const {
    return *reinterpret_cast<const T *>(m_val);
  }

  std::size_t bytes_size() const { return m_sz; }

  addr_t val_addr() const { return m_val; }

  void free_heap() const { return m_destructor(m_val); }

  void move() { return m_move(m_val, m_a); }

private:
  addr_t m_a;
  addr_t m_val;
  std::function<void(addr_t)> m_destructor;
  std::function<void(addr_t, addr_t)> m_move;
  std::size_t m_sz;
};

class WriteSet {
public:
  virtual void update(const WriteOp &op) = 0;
  virtual std::optional<value_addr_t> find_opt(const WriteOp &op) const = 0;
  std::optional<value_addr_t> find_opt(const WriteOp &&op) const {
    return find_opt(op);
  }
};

class WriteSetCompare {
public:
  bool operator()(const WriteOp &a, const WriteOp &b) const {
    return a.addr() < b.addr();
  }
};
class WriteOrderedSet : public std::set<WriteOp, WriteSetCompare>,
                        public WriteSet {
public:
  using Set = std::set<WriteOp, WriteSetCompare>;
  WriteOrderedSet() : Set(), WriteSet() {}
  std::optional<value_addr_t> find_opt(const WriteOp &op) const override {
    const auto itr = Set::find(op);
    if (itr == end()) {
      return std::nullopt;
    }
    return std::optional(itr->val_addr());
  }

  void update(const WriteOp &op) override {
    const auto itr = Set::find(op);
    if (itr == end()) {
      Set::insert(op);
    } else {
      Set::erase(itr);
      Set::insert(op);
    }
  }
};
} // namespace tl2::internal
