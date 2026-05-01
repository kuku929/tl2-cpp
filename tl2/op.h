#pragma once
#include "ankerl/unordered_dense.h"
#include "hash_table.h"
#include "ska_sort.h"
#include "types.h"
#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <set>
#include <thread>
#include <type_traits>
#include <vector>

namespace tl2::internal {
using namespace tl2::internal;
using value_addr_t = addr_t;

class AbstractLocationSet;

class WriteVal {
public:
  WriteVal()
      : m_val(0), m_destructor([](addr_t obj) noexcept {}),
        m_move([](addr_t src, addr_t dest) noexcept {}), m_sz(0u) {
    ;
  }
  template <typename T>
  [[nodiscard]]
  WriteVal(const T *v)
      : m_val(to_addr(v)), m_destructor([](addr_t obj) noexcept -> void {
          reinterpret_cast<T *>(obj)->~T();
          return;
        }),
        m_move([](addr_t src, addr_t dest) noexcept -> void {
          *reinterpret_cast<T *>(dest) = std::move(*reinterpret_cast<T *>(src));
        }),
        m_sz(sizeof(T)) {
    ;
  }

  template <typename T> [[nodiscard]] T &value() const {
    return *reinterpret_cast<T *>(m_val);
  }

  __attribute__((always_inline)) std::size_t bytes_size() const { return m_sz; }

  __attribute__((always_inline)) value_addr_t val_addr() const { return m_val; }

  __attribute__((always_inline)) void free_heap() const {
    return m_destructor(m_val);
  }

  __attribute__((always_inline)) void move(addr_t dest) {
    return m_move(m_val, dest);
  }

private:
  value_addr_t m_val;
  using destructor_t = void (*)(addr_t) noexcept;
  using move_t = void (*)(addr_t, addr_t) noexcept;

  destructor_t m_destructor;
  move_t m_move;
  // std::function<void(addr_t)> m_destructor;
  // std::function<void(addr_t, addr_t)> m_move;
  std::size_t m_sz;
};

class Location {
public:
  Location(addr_t addr)
      : write_val(), m_addr(addr), m_rv(0), m_read(false), m_written(false),
        m_owner() {
    ;
  }
  Location(addr_t addr, bool read)
      : write_val(), m_addr(addr), m_rv(0), m_read(false), m_written(false),
        m_owner() {
    if (read)
      this->read();
  }

  template <typename T, typename StorePolicy>
  Location(addr_t addr, T &&write_val, StorePolicy &store)
      : m_addr(addr), m_rv(0), m_read(false), m_written(false), m_owner() {
    write(std::forward<T>(write_val), store);
  }

  void read() const {
    m_read = true;
    // We delay the read as much as possible
    // This allows us to abort in case
    // a write occurs in the time the initial
    // get() was called and we added to the set.
    m_rv = get_version();
  }

  template <typename T, typename StorePolicy>
  void write(T &&val, StorePolicy &store) const {
    if (m_written) {
      // Address already in write-set: overwrite existing staged value.
      write_val.value<T>() = std::forward<T>(val);
      return;
    }
    // First write for this address in this transaction: stage a copied value.
    T *obj = static_cast<T *>(store.allocate(sizeof(T), alignof(T)));
    if constexpr (std::is_trivial_v<T>) {
      // fast path for trivial data types.
      std::memcpy(obj, &val, sizeof(T));
    } else {
      new (obj) T(std::forward<T>(val));
    }
    write_val = WriteVal(obj);
    m_written = true;
  }

  __attribute__((always_inline)) void move() const { write_val.move(m_addr); }

  inline bool lock() const {
    if (std::this_thread::get_id() == m_owner)
      return false;
    hashtbl[m_addr].lock();
    m_owner = std::this_thread::get_id();
    return true;
  }

  inline VersionLock *lock_ptr() const { return &hashtbl[m_addr]; }

  inline void mark_owner() const { m_owner = std::this_thread::get_id(); }

  inline version_t get_version() const {
    if (std::this_thread::get_id() == m_owner)
      return hashtbl[m_addr].unsafe_get_version();
    return hashtbl[m_addr].get_version();
  }

  inline void set_version(version_t v) const {
    if (std::this_thread::get_id() == m_owner)
      return hashtbl[m_addr].unsafe_set_version(v);
    return hashtbl[m_addr].set_version(v);
  }

  inline void unlock() const {
    hashtbl[m_addr].unlock();
    m_owner = std::thread::id();
  }

  bool was_read() const { return m_read; }
  bool was_written() const { return m_written; }
  addr_t addr() const { return m_addr; }
  mutable WriteVal write_val; // the value last written
private:
  addr_t m_addr;
  mutable version_t m_rv;          // last read version
  mutable bool m_read;             // whether this location was read
  mutable bool m_written;          // whethere this location was written to
  mutable std::thread::id m_owner; // whether write lock is held or not
};

class AbstractLocationSet {
public:
  template <typename T, typename StorePolicy>
  void register_write(addr_t addr, T &&val, StorePolicy &store);
  virtual void register_read(addr_t addr) = 0;
  virtual std::optional<value_addr_t> find_write(addr_t addr) const = 0;
  virtual void stable_sort() = 0;
  virtual std::size_t size() const = 0;
  virtual void clear() = 0;
};

class LocationSetCompare {
public:
  bool operator()(const Location &a, const Location &b) const {
    return a.addr() < b.addr();
  }
};
class LocationOrderedSet : public std::set<Location, LocationSetCompare>,
                           public AbstractLocationSet {
public:
  using Set = std::set<Location, LocationSetCompare>;

  LocationOrderedSet() : Set(), AbstractLocationSet() {}

  std::optional<value_addr_t> find_write(addr_t addr) const override {
    std::optional<value_addr_t> result(std::nullopt);
    if (const auto itr = Set::find(Location(addr));
        itr != Set::end() && itr->was_written()) {
      result = itr->write_val.val_addr();
    }
    return result;
  }

  template <typename T, typename StorePolicy>
  void register_write(addr_t addr, T &&val, StorePolicy &store) {
    Location loc(addr);
    if (Set::iterator itr = Set::find(loc); itr != Set::end()) {
      itr->write(std::forward<T>(val), store);
      return;
    }
    // since a write is costly we avoid it.
    loc.write(std::forward<T>(val), store);
    Set::insert(loc);
  }

  void register_read(addr_t addr) override {
    const auto &[itr, inserted] = Set::insert(Location(addr, true));
    if (!inserted) {
      itr->read();
      return;
    }
  }

  std::size_t size() const override { return Set::size(); }

  void clear() override { Set::clear(); }

  void stable_sort() override { return; }
};

class LocationHashVectorSet : public AbstractLocationSet {
public:
  LocationHashVectorSet() = default;

  template <typename T, typename StorePolicy>
  void register_write(addr_t addr, T &&val, StorePolicy &store) {
    if (const auto itr = m_map.find(addr); itr != m_map.end()) {
      m_vec[itr->second].write(std::forward<T>(val), store);
    } else {
      m_map.emplace(addr, m_vec.size());
      m_vec.emplace_back(Location(addr, std::forward<T>(val), store));
    }
  }

  void register_read(addr_t addr) override {
    if (const auto itr = m_map.find(addr); itr != m_map.end()) {
      m_vec[itr->second].read();
    } else {
      m_map.emplace(addr, m_vec.size());
      m_vec.emplace_back(Location(addr, true));
    }
  }

  void stable_sort() override {
    return ska_sort(
        m_vec.begin(), m_vec.end(),
        [](const Location &loc) noexcept -> addr_t { return loc.addr(); });
  }

  std::optional<value_addr_t> find_write(addr_t addr) const override {
    std::optional<value_addr_t> result(std::nullopt);
    if (const auto itr = m_map.find(addr);
        itr != m_map.end() && m_vec[itr->second].was_written()) {
      result = m_vec[itr->second].write_val.val_addr();
    }
    return result;
  }

  std::size_t size() const override { return m_vec.size(); }

  auto begin() const { return m_vec.begin(); }

  auto end() const { return m_vec.end(); }

  void clear() override {
    m_vec.clear();
    m_map.clear();
  }

private:
  std::vector<Location> m_vec;
  ankerl::unordered_dense::map<addr_t, size_t> m_map;
};

} // namespace tl2::internal
