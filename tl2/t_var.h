#pragma once
#include "log.h"
#include "state.h"
#include "types.h"
#include "version_lock.h"
#include <cassert>
#include <iostream>
#include <memory_resource>
#include <thread>
#include <type_traits>

// TODO: Any way to detect invalid get/set at compile time?
namespace tl2 {
using namespace tl2::internal;
template <tl2::internal::Constructible T> class TVar {
public:
  TVar(T data) : m_data(std::move(data)) {}

  explicit operator T() const {
    manager.assert_in_transaction();
    log.append_read(&m_data);
    std::optional<T> entry;
    if ((entry = log.value_at(&m_data)).has_value())
      return entry.value();
    return m_data;
  }

  TVar &operator=(const T val) {
    manager.assert_in_transaction();
    log.append_write(&m_data, val);
    return *this;
  }

private:
  T m_data;
};
} // namespace tl2
