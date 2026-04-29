#pragma once
#include "types.h"
#include <set>
#include <tuple>

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
class ReadOrderedSet : public std::set<ReadOp, ReadSetCompare>, public AbstractReadSet {
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
} // namespace tl2::internal
