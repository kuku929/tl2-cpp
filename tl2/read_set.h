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

class ReadSet {
public:
  virtual void clear() = 0;
  virtual void insert(const ReadOp &op) = 0;
  virtual void modify(const ReadOp &op) = 0;
  virtual bool contains(const ReadOp &op) = 0;
};

class ReadSetCompare {
public:
  inline bool operator()(const ReadOp &a, const ReadOp &b) const {
    return a.addr() < b.addr();
  }
};
class ReadOrderedSet : public std::set<ReadOp, ReadSetCompare>, public ReadSet {
public:
  using Set = std::set<ReadOp, ReadSetCompare>;
  ReadOrderedSet() : Set(), ReadSet() {}
  void clear() override { Set::clear(); }

  void insert(const ReadOp &op) override { Set::insert(op); }

  void modify(const ReadOp &op) override {
    // modify is called when address is already
    // present. No speedup right now.
    Set::erase(op);
    Set::insert(op);
  }

  bool contains(const ReadOp &op) override {
    return Set::contains(op);
  }
};
} // namespace tl2::internal
