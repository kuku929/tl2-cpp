#pragma once
#include "log.h"
#include "state.h"
#include "types.h"
#include "version_lock.h"
#include <cassert>
#include <iostream>
#include <memory_resource>
#include <thread>

// TODO: Any way to detect invalid get/set at compile time?
namespace tl2 {
using namespace tl2::internal;
template <typename T> class TVar {
public:
  TVar(T data) : m_data(std::move(data)) {}

  explicit operator T() const {
    manager.assert_in_transaction();
    const T *address = &m_data;
    // NOTE: can we prevent copying here?
    T val = m_data;
    std::optional<T> entry;
    if ((entry = log.value_at(address)).has_value())
      val = entry.value();
    log.append_read(address);
    return val;
  }

  TVar& operator=(T &val) {
    manager.assert_in_transaction();
    if (&val == &m_data)
      return *this;
    log.append_write(&m_data, val);
    return *this;
  }

private:
  T m_data;
};
} // namespace tl2
