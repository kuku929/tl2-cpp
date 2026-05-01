#pragma once
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace tl2::internal {
using uint = std::uint32_t;
using version_t = std::uint64_t;
using addr_t = std::uintptr_t;

#define to_addr(x) reinterpret_cast<addr_t>(x)

template <typename T>
concept Constructible = std::move_constructible<T>;
} // namespace tl2::internal
