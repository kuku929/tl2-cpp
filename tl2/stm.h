#pragma once
#include "lock_guard.h"
#include "log.h"
#include "state.h"
#include "t_var.h"
#include <cstring>
#include <tuple>
namespace tl2 {
using namespace tl2::internal;

template <typename Transaction> inline void atomically(Transaction t);
template <typename Transaction, typename... Args>
inline void atomically(Transaction t, Args&... args);
namespace internal {
void commit(version_t write_version);
bool try_commit(const version_t read_version);

inline void commit(version_t write_version) {
  // One optimization is to construct the write
  // set when we construct the lock guard and use
  // it here.
  for (const Location &loc : log.locations()) {
    // first bump up the version so that other threads
    // can abort if needed.
    if (!loc.was_written())
      continue;
    loc.set_version(write_version);
    loc.move();
  }
}

inline bool try_commit() {
  auto guard = make_lock_guard(log.locations());
  const auto write_version = global_clock.incr_version();
  if (write_version == manager.read_version() + 1) {
    // no other thread has made changes commit
    commit(write_version);
    return true;
  }
  for (const Location &loc : log.locations()) {
    if (!loc.was_read())
      continue;
    if (loc.get_version() > manager.read_version()) {
      return false;
    }
  }
  commit(write_version);
  return true;
};
} // namespace internal
template <typename Transaction> inline void atomically(Transaction t) {
  while (true) {
    manager.start_transaction();
    t();
    if (try_commit())
      break;
  }
  manager.end_transaction();
}

template <typename Transaction, typename... Args>
inline void atomically(Transaction t, Args&... args) {
  const auto saved = std::make_tuple(args...);
  while (true) {
    std::apply(
        [&](const auto&... saved_args) {
            ((args = saved_args), ...);
        },
        saved
    );
    manager.start_transaction();
    t(args...);
    if (try_commit())
      break;
  }
  manager.end_transaction();
}
}; // namespace tl2
