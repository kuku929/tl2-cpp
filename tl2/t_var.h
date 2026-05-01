#pragma once
#include "hash_table.h"
#include "log.h"
#include "state.h"
#include "types.h"
#include "version_lock.h"
#include <cassert>
#include <iostream> // DEBUG
#include <memory_resource>
#include <thread>
#include <type_traits>

namespace tl2 {
using namespace tl2::internal;
template <tl2::internal::Constructible T> class TVar {
public:
  TVar() : m_data() {}
  TVar(T data) : m_data(std::move(data)) {}
  TVar(const TVar<T> &other) noexcept { this->m_data = other.m_data; }
  TVar(TVar<T> &&other) noexcept { this->m_data = std::move(other.m_data); }
  // Otherwise TSan will flag optimistic read as a data race.
  __attribute__((no_sanitize_thread)) explicit operator T() const {
    if constexpr (std::is_default_constructible_v<T>) {
      /*
      We do it this way to encourage the compiler to
      perform copy elision. There is only one copy
      assignment happening here.
      */
      manager.assert_in_transaction();
      auto &&log = manager.log;
      log.append_read(&m_data);
      T result;
      if (std::optional<T *> entry = log.value_at(&m_data); entry.has_value()) {
        result = *entry.value();
      } else {
        if constexpr (std::is_trivial_v<T>) {
          // fast path for trivial data types.
          result = m_data;
        } else {
          VersionLock &mutex = hashtbl[to_addr(&m_data)];
          // default to using locks, beats the point of a STM.
          mutex.lock();
          result = m_data;
          mutex.unlock();
        }
      }
      return result;
    } else {
      /*
      Since T is not default constructible, fallback
      to a slower path.
      */
      manager.assert_in_transaction();
      auto &&log = manager.log;
      log.append_read(&m_data);
      if (std::optional<T *> entry = log.value_at(&m_data); entry.has_value()) {
        return *entry.value();
      } else {
        if constexpr (std::is_trivial_v<T>) {
          // fast path for trivial data types.
          return m_data;
        } else {
          VersionLock &mutex = hashtbl[to_addr(&m_data)];
          // default to using locks, beats the point of a STM.
          mutex.lock();
          T result = m_data;
          mutex.unlock();
          return result;
        }
      }
    }
  }

  T unsafe_get() {
    return m_data;
  }

  TVar &operator=(T val) {
    manager.assert_in_transaction();
    auto &&log = manager.log;
    /*
    If lvalue is provided a copy is created
    If rvalue is provided the compiler will optimize
    to not copy. Thus we can call std::move()
    */
    log.append_write(&m_data, std::move(val));
    return *this;
  }

  TVar<T> &operator=(const TVar<T> &other) {
    if (&other == this) {
      return *this;
    }
    // will add to read and write set
    return (*this = static_cast<T>(other));
  }

private:
  T m_data;
};
} // namespace tl2
