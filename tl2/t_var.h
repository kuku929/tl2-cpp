#pragma once
#include "hash_table.h"
#include "log.h"
#include "state.h"
#include "types.h"
#include "version_lock.h"
#include <cassert>
#include <iostream>
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
  explicit operator T() const {
    /*
    We do it this way to encourage the compiler to
    perform copy elision. There is only one copy
    assignment happening here.
    */
    manager.assert_in_transaction();
    log.append_read(&m_data);
    T result;
    VersionLock &mutex = hashtbl[to_addr(&m_data)];
    if (std::optional<T *> entry = log.value_at(&m_data); entry.has_value())
      result = *entry.value();
    else {
      mutex.lock();
      result = m_data;
      mutex.unlock();
    }
    return result;
  }

  TVar &operator=(T val) {
    manager.assert_in_transaction();
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
