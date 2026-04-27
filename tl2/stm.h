#pragma once
#include "log.h"
#include "state.h"
#include "t_var.h"
#include <cstring>
namespace tl2 {
using namespace tl2::internal;

template <typename Transaction> inline void atomically(Transaction t);
namespace internal {
void commit(version_t write_version);
bool try_commit(const version_t read_version);

inline void commit(version_t write_version) {
  for (auto op : log.writes()) {
    std::memcpy(reinterpret_cast<void *>(op.addr()),
                reinterpret_cast<void *>(op.val_addr()), op.bytes_size());
    hashtbl[op.addr()].unsafe_set_version(write_version);
  }
}

inline bool try_commit() {
  auto guard = make_lock_guard(
      log.writes().begin(), log.writes().end(),
      [](const WriteOp &op) -> VersionLock & { return hashtbl[op.addr()]; });
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
  return true;
};
} // namespace internal
// need some machinery to deduce lambda return types and so on...
template <typename Transaction> inline void atomically(Transaction t) {
  while (true) {
    manager.start_transaction();
    t();
    if (try_commit())
      break;
  }
}
}; // namespace tl2
