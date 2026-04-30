#pragma once
#include "types.h"
#include <ankerl/unordered_dense.h>
#include <set>
#include <tuple>
#include <vector>

namespace tl2::internal {
using namespace tl2::internal;
class ReadOp {
public:
  ReadOp(addr_t a, version_t v) : m_a(std::move(a)), m_v(std::move(v)) { ; }
  inline addr_t addr() const { return m_a; }
  inline version_t version() const { return m_v; }

private:
  addr_t m_a;
  version_t m_v;
};

class AbstractReadSet {
public:
  virtual void clear() = 0;
  virtual void update(const ReadOp &op) = 0;
};

class ReadSetCompare {
public:
  inline bool operator()(const ReadOp &a, const ReadOp &b) const {
    return a.addr() < b.addr();
  }
};
class ReadOrderedSet : public std::set<ReadOp, ReadSetCompare>,
                       public AbstractReadSet {
public:
  using Set = std::set<ReadOp, ReadSetCompare>;
  ReadOrderedSet() : Set(), AbstractReadSet() {}
  void clear() override { Set::clear(); }

  void update(const ReadOp &op) override {
    const auto itr = Set::find(op);
    if (itr == end()) {
      Set::insert(op);
    } else {
      Set::erase(itr);
      Set::insert(op);
    }
  }
};

class ReadVectorSet : public AbstractReadSet {
public:
  ReadVectorSet() = default;

  void clear() override { m_vec.clear(); }

  void update(const ReadOp &op) override { m_vec.push_back(op); }

  std::size_t size() const { return m_vec.size(); }

  auto begin() const { return m_vec.begin(); }
  auto end() const { return m_vec.end(); }

private:
  std::vector<ReadOp> m_vec;
};

class ReadHashVectorSet : public AbstractReadSet {
public:
  ReadHashVectorSet() = default;

  void clear() override {
    m_vec.clear();
    m_map.clear();
  }

  void update(const ReadOp &op) override {
    const addr_t a = op.addr();
    auto it = m_map.find(a);
    if (it == m_map.end()) {
      m_map.emplace(a, m_vec.size());
      m_vec.push_back(op);
    } else {
      m_vec[it->second] = op;
    }
  }

  std::size_t size() const { return m_vec.size(); }

  auto begin() const { return m_vec.begin(); }
  auto end() const { return m_vec.end(); }

private:
  std::vector<ReadOp> m_vec;
  ankerl::unordered_dense::map<addr_t, size_t> m_map;
};
} // namespace tl2::internal
