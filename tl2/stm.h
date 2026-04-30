#pragma once
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
  for (auto op : log.writes()) {
    // first bump up the version so that other threads
    // can abort if needed.
    hashtbl[op.addr()].unsafe_set_version(write_version);
    op.move();
  }
}

inline bool try_commit() {
  if (log.writes().size() != 0){
    auto guard =
        make_lock_guard(log.writes(), [](const WriteOp &op) -> VersionLock & {
          return hashtbl[op.addr()];
        });
    const auto write_version = global_clock.incr_version();
    if (write_version == manager.read_version() + 1) {
      // no other thread has made changes commit
      commit(write_version);
      return true;
    }
    for (auto op : log.reads()) {
      if (hashtbl[op.addr()].unsafe_get_version() > manager.read_version()) {
        return false;
      }
    }
    commit(write_version);
  }
  else{
    if (global_clock.unsafe_get_version() == manager.read_version()) {
      return true;
    }
    for (auto op : log.reads()) {
      if (hashtbl[op.addr()].unsafe_get_version() > manager.read_version()) {
        return false;
      }
    }
  }
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
