#pragma once
#include <concepts>
#include <cstdint>

namespace tl2::internal {
using uint = std::uint32_t;
using version_t = std::uint64_t;
using addr_t = std::uintptr_t;

#define to_addr(x) reinterpret_cast<addr_t>(x)

template <typename T>
concept Constructible = std::move_constructible<T>;
template <typename T> constexpr bool is_primitive_v = std::is_fundamental_v<T>;
} // namespace tl2::internal
