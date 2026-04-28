#pragma once
#include "types.h"
#include "ankerl/unordered_dense.h"
#include "function.h"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <set>
#include <vector>

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
  func::function<void(addr_t)> m_destructor;
  func::function<void(addr_t, addr_t)> m_move;
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

class WriteHashVectorSet : public WriteSet {
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

  std::optional<value_addr_t> find_opt(const WriteOp &op) const override {
    auto it = m_map.find(op.addr());
    if (it == m_map.end()) {
      return std::nullopt;
    }
    return std::optional(m_vec[it->second].val_addr());
  }

  void clear() {
    m_vec.clear();
    m_map.clear();
  }

  auto begin() {
    sort_if_needed();
    return m_vec.begin();
  }
  auto end() {
    sort_if_needed();
    return m_vec.end();
  }

  auto begin() const {
    const_cast<WriteHashVectorSet *>(this)->sort_if_needed();
    return m_vec.begin();
  }
  auto end() const {
    const_cast<WriteHashVectorSet *>(this)->sort_if_needed();
    return m_vec.end();
  }

private:
  void sort_if_needed() {
    std::sort(m_vec.begin(), m_vec.end(),
              [](const WriteOp &a, const WriteOp &b) {
                return a.addr() < b.addr();
              });
    m_map.clear();
    for (size_t i = 0; i < m_vec.size(); ++i) {
      m_map.emplace(m_vec[i].addr(), i);
    }
  }

  std::vector<WriteOp> m_vec;
  ankerl::unordered_dense::map<addr_t, size_t> m_map;
};

} // namespace tl2::internal
