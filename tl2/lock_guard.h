#pragma once
#include "op.h"
#include "version_lock.h"

namespace tl2::internal {
using namespace tl2::internal;
template <typename LocationSet> class LockGuard {
public:
  LockGuard(LocationSet &l) {
    l.stable_sort();
    m_locked.reserve(l.size());
    for (auto &loc : l) {
      if (loc.lock())
        m_locked.push_back(&loc);
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
  std::vector<const Location *> m_locked;
};

template <typename LocationSet> auto make_lock_guard(LocationSet &l) {
  return LockGuard<LocationSet>(l);
}
} // namespace tl2::internal