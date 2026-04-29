#pragma once
#include "types.h"
#include "ankerl/unordered_dense.h"
// #include "function.h"
// #include <functional>
#include "ska_sort.h"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <set>
#include <vector>

namespace tl2::internal {
using namespace tl2::internal;
using value_addr_t = addr_t;
class AbstractWriteSet;

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
   __attribute__((always_inline)) addr_t addr() const { return m_a; }

  template <typename T> [[nodiscard]] const T value() const {
    return *reinterpret_cast<const T *>(m_val);
  }

  __attribute__((always_inline)) std::size_t bytes_size() const { return m_sz; }

  __attribute__((always_inline)) addr_t val_addr() const { return m_val; }

  __attribute__((always_inline)) void free_heap() const { return m_destructor(m_val); }

  __attribute__((always_inline)) void move() { return m_move(m_val, m_a); }

private:
  addr_t m_a;
  addr_t m_val;
  using destructor_t = void(*)(addr_t) noexcept;
  using move_t = void(*)(addr_t, addr_t) noexcept;

  destructor_t m_destructor;
  move_t m_move;
  // std::function<void(addr_t)> m_destructor;
  // std::function<void(addr_t, addr_t)> m_move;
  std::size_t m_sz;
};

class AbstractWriteSet {
public:
  virtual void update(const WriteOp &op) = 0;
  virtual std::optional<value_addr_t> find_opt(const WriteOp &op) const = 0;
  virtual void stable_sort() = 0;
  virtual std::size_t size() const = 0;
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
                        public AbstractWriteSet {
public:
  using Set = std::set<WriteOp, WriteSetCompare>;
  using Set::size;
  WriteOrderedSet() : Set(), AbstractWriteSet() {}
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

  std::size_t size() const override {
    return Set::size();
  }

  void stable_sort() override {
    return;
  }
};

class WriteHashVectorSet : public AbstractWriteSet {
public:
  WriteHashVectorSet() = default;

  void update(const WriteOp &op) override {
    const addr_t a = op.addr();
    auto it = m_map.find(a);
    if (it == m_map.end()) {
      m_map.emplace(a, m_vec.size());
      m_vec.push_back(op);
    } else {
      m_vec[it->second] = op;
    }
  }

  void stable_sort() override {
    return ska_sort(m_vec.begin(), m_vec.end(), [](const WriteOp &op) noexcept -> addr_t {
      return op.addr();
    });
  }

  std::optional<value_addr_t> find_opt(const WriteOp &op) const override {
    auto it = m_map.find(op.addr());
    if (it == m_map.end()) {
      return std::nullopt;
    }
    return std::optional(m_vec[it->second].val_addr());
  }

  std::size_t size() const override {
    return m_vec.size();
  }

  auto begin() const {
    return m_vec.begin();
  }

  auto end() const {
    return m_vec.end();
  }

  void clear() {
    m_vec.clear();
    m_map.clear();
  }

private:
  std::vector<WriteOp> m_vec;
  ankerl::unordered_dense::map<addr_t, size_t> m_map;
};

} // namespace tl2::internal
