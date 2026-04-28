#pragma once
#include <concepts>
#include <cstdint>

namespace tl2::internal {
using uint = std::uint32_t;
using version_t = std::uint64_t;
using addr_t = std::uintptr_t;

template <typename T>
concept Constructible = std::move_constructible<T>;
} // namespace tl2::internal
