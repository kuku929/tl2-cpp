#pragma once
#include "types.h"
#include "write_set.h"
#include <atomic>
#include <thread>
#include <unordered_set>
#include <vector>

namespace tl2::internal {
using namespace tl2::internal;
class VersionLock {
public:
  version_t get_version() const {
    while (true) {
      // TODO: is this fast
      const auto s = m_state.load(std::memory_order_acquire);
      if ((s & kLocked) == 0)
        return unpack_version(s);
      std::this_thread::yield();
    }
  }

  version_t unsafe_get_version() const {
    return unpack_version(m_state.load(std::memory_order_relaxed));
  }

  version_t incr_version() {
    const auto new_state =
        m_state.fetch_add(kVersionInc, std::memory_order_acq_rel) + kVersionInc;
    return unpack_version(new_state);
  }

  void set_version(version_t v) {
    while (true) {
      auto s = m_state.load(std::memory_order_acquire);
      if (s & kLocked) {
        std::this_thread::yield();
        continue;
      }
      if (m_state.compare_exchange_weak(s, pack(v, false),
                                        std::memory_order_release,
                                        std::memory_order_acquire))
        return;
    }
  }

  void unsafe_set_version(version_t v) {
    const auto s = m_state.load(std::memory_order_relaxed);
    const bool locked = (s & kLocked) != 0;
    m_state.store(pack(v, locked), std::memory_order_release);
  }

  void lock() {
    while (true) {
      auto s = m_state.load(std::memory_order_acquire);
      if (s & kLocked) {
        std::this_thread::yield();
        continue;
      }
      if (m_state.compare_exchange_weak(s, s | kLocked,
                                        std::memory_order_acquire,
                                        std::memory_order_relaxed))
        return;
    }
  }

  void unlock() { m_state.fetch_and(~kLocked, std::memory_order_release); }

private:
  static constexpr version_t kLocked = 1;
  static constexpr version_t kVersionInc = 2;

  static constexpr version_t pack(version_t v, bool locked) {
    return (v << 1) | (locked ? 1 : 0);
  }

  static constexpr version_t unpack_version(version_t s) { return s >> 1; }

  std::atomic<version_t> m_state{0};
};

template <std::size_t N> struct Pad {
  char data[N];
};
template <> struct Pad<0> {};

#if defined(__GCC_DESTRUCTIVE_SIZE)
static constexpr std::size_t CACHE_LINE_SIZE = __GCC_DESTRUCTIVE_SIZE;
#elif defined(__cpp_lib_hardware_interference_size)
static constexpr std::size_t CACHE_LINE_SIZE =
    std::hardware_destructive_interference_size;
#else
static constexpr std::size_t CACHE_LINE_SIZE = 64;
#endif

static constexpr std::size_t lock_size = sizeof(VersionLock);
#if __cpp_static_assert >= 202306L
static_assert(lock_size <= CACHE_LINE_SIZE,
              "Lock object does not fit in cache!");
#else
static_assert(lock_size <= CACHE_LINE_SIZE);
#endif
static constexpr std::size_t pad_size =
    (CACHE_LINE_SIZE - (lock_size % CACHE_LINE_SIZE)) % CACHE_LINE_SIZE;

class alignas(CACHE_LINE_SIZE) PaddedVersionLock : public VersionLock {
public:
  using VersionLock::VersionLock;

private:
  Pad<pad_size> pad;
} static global_clock;

template <typename WriteSet, typename LockSelector> class LockGuard {
public:
  LockGuard(WriteSet &w, LockSelector lock_selector) {
    w.stable_sort();
    m_locked.reserve(w.size());
    std::unordered_set<VersionLock *> seen;
    for (const WriteOp &op : w) {
      VersionLock *lock = &lock_selector(op);
      if (seen.insert(lock).second) {
        lock->lock();
        m_locked.push_back(lock);
      }
    }
  }

  ~LockGuard() {
    for (auto it = m_locked.rbegin(); it != m_locked.rend(); ++it) {
      (*it)->unlock();
    }
  }

  LockGuard(const LockGuard &) = delete;
  LockGuard &operator=(const LockGuard &) = delete;
  LockGuard(LockGuard &&) = delete;
  LockGuard &operator=(LockGuard &&) = delete;

private:
  std::vector<VersionLock *> m_locked;
};

template <typename WriteSet, typename LockSelector>
auto make_lock_guard(WriteSet &w, LockSelector lock_selector) {
  return LockGuard<WriteSet, LockSelector>(w, lock_selector);
}
} // namespace tl2::internal
