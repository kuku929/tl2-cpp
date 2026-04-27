#pragma once
#include <cstddef>
#include <memory_resource>

namespace tl2::internal {
	// Single shared synchronized pool resource for those logs that are created
	// with an external resource (e.g., to share allocations across threads).
	inline std::pmr::synchronized_pool_resource synchronized_pool_resource{};
} // namespace tl2::internal