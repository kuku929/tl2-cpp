#pragma once
#include "lock_guard.h"
#include "log.h"
#include "state.h"
#include "t_var.h"
#include <cstring>
#include <exception>
#include <tuple>
#include <thread>
#include <type_traits>
namespace tl2 {
using namespace tl2::internal;

template <typename T> struct is_t_var : std::false_type {};
template <typename T> struct is_t_var<TVar<T>> : std::true_type {};
// Source - https://stackoverflow.com/a/67315413
// Posted by songyuanyao
// Retrieved 2026-05-01, License - CC BY-SA 4.0

template <typename T> inline constexpr bool is_t_var_v = is_t_var<T>::value;

namespace internal {
void commit(version_t write_version);
bool try_commit(const version_t read_version);

inline void commit(version_t write_version) {
  // One optimization is to construct the write
  // set when we construct the lock guard and use
  // it here.
  auto &&log = manager.log;
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
  auto &&log = manager.log;
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
}

template <typename... Args>
inline void restore_args(const auto &saved, Args &...args) {
  std::apply(
      [&](const auto &...saved_args) noexcept {
        // Only restore arguments that are not TVars and
        // are references.
        (
            [&]() noexcept {
              if constexpr (!tl2::is_t_var_v<Args>) {
                (args = saved_args);
              }
            }(),
            ...);
      },
      saved);
}

template <typename... T>
inline constexpr bool are_copy_assignable_v =
    (std::is_copy_assignable_v<std::remove_reference_t<T>> && ...);

} // namespace internal
inline bool in_transaction() { return manager.in_transaction(); }

template <typename Transaction, typename... Args>
inline auto atomically(Transaction &&t, Args &...args)
    -> decltype(std::forward<Transaction>(t)()) {
  static_assert(tl2::internal::are_copy_assignable_v<Args...>,
                "Restored arguments must be copy assignable");
  using ReturnType = decltype(t());
  if (in_transaction()) {
    /*
    Nested atomic calls can be flattened to a single big transaction.
    If we are nested then atomically() simple executes t().
    We don't need to worry about restoring these arguments. because:
      Case I : the arguments are outside the parent transaction
        It is the parent transaction that will restore the args if need be.
      Case II : arguments are inside the parent transaction
        In the case of abort, the stack rewinds and we start anew so these
        variables are "automatically" restored.
    */
    return t();
  }
  const auto saved = std::make_tuple(args...);
  try { // try{} is zero-cost
    if constexpr (!std::is_void_v<ReturnType>) {
      std::optional<ReturnType> ret;
      while (true) {
        manager.start_transaction();
        ret = t();
        if (try_commit())
          break;
        tl2::internal::restore_args(saved, args...);
      }
      manager.end_transaction();
      return ret.value();
    } else {
      while (true) {
        manager.start_transaction();
        t();
        if (try_commit())
          break;
        tl2::internal::restore_args(saved, args...);
      }
      manager.end_transaction();
    }
  } catch (const std::exception &e) {
    std::cerr << "Exception occured during transaction: " << e.what()
              << std::endl;
    manager.end_transaction();
    throw;
  }
}
} // namespace tl2