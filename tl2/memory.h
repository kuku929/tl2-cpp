#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <memory_resource>
#include <new>

namespace tl2::internal {

	enum class AllocationStrategy {
		PerTransactionBuffer,
		SynchronizedPool,
	};

	inline std::atomic<AllocationStrategy> allocation_strategy{AllocationStrategy::PerTransactionBuffer};

	inline std::pmr::synchronized_pool_resource synchronized_pool_resource{};

	inline void set_allocation_strategy(const AllocationStrategy strategy) {
		allocation_strategy.store(strategy, std::memory_order_release);
	}

	inline AllocationStrategy get_allocation_strategy() {
		return allocation_strategy.load(std::memory_order_acquire);
	}

	inline std::unique_ptr<std::pmr::monotonic_buffer_resource>	make_transaction_resource(std::byte *local_buffer, const std::size_t local_buffer_size) {
		if (get_allocation_strategy() == AllocationStrategy::SynchronizedPool) {
			// This is weird to do, but I only need to change one line here for testing
			return std::make_unique<std::pmr::monotonic_buffer_resource>(&synchronized_pool_resource);
		}
		else {
			return std::make_unique<std::pmr::monotonic_buffer_resource>(local_buffer, local_buffer_size);
		}
	}

} // namespace tl2::internal