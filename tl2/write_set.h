#pragma once
#include "types.h"
#include <memory>
#include <optional>
#include <set>

namespace tl2::internal {
using namespace tl2::internal;

class WriteSet;

class WriteOp {
public:
  WriteOp(addr_t a) : m_a(std::move(a)), m_val(0), m_sz(0) { ; }
  template <typename T> WriteOp(const T *a, const T *v) : m_a(reinterpret_cast<addr_t>(a)), m_val(reinterpret_cast<addr_t>(v)), m_sz(sizeof(T)) { ; }
  addr_t addr() const { return m_a; }

  template <typename T> const T value() const {
    return *static_cast<const T *>(m_val);
  }

  addr_t val_addr() const { return m_val; }

  std::size_t bytes_size() const { return m_sz; }

private:
  addr_t m_a;
  addr_t m_val;
  std::size_t m_sz;
};

class WriteSet {
public:
  virtual void insert(const WriteOp &op) = 0;
  virtual void modify(const WriteOp &op) = 0;
  virtual std::optional<addr_t> find_opt(const WriteOp &op) const = 0;
  std::optional<addr_t> find_opt(const WriteOp &&op) const {
    return find_opt(op);
  }
  virtual bool contains(const WriteOp &op) = 0;
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
  std::optional<addr_t> find_opt(const WriteOp &op) const override {
    const auto itr = Set::find(op);
    if (itr == end()) {
      return std::nullopt;
    }
    return std::optional(itr->val_addr());
  }

  void insert(const WriteOp &op) override { Set::insert(op); }

  void modify(const WriteOp &op) override {
    // modify is called when address is already
    // present. No speedup right now.
    erase(op);
    insert(op);
  }

  bool contains(const WriteOp &op) override {
    return Set::contains(op);
  }
};
} // namespace tl2::internal
