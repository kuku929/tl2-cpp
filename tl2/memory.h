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

	struct LogResourceHandle {
		std::pmr::memory_resource *resource{nullptr};
		std::unique_ptr<std::pmr::monotonic_buffer_resource> owned_resource{};
	};

	template <AllocationStrategy AS>
	inline LogResourceHandle make_log_resource(std::byte *local_buffer, const std::size_t local_buffer_size) {
		if constexpr (AS == AllocationStrategy::SynchronizedPool) {
			return LogResourceHandle{&synchronized_pool_resource, nullptr};
		} else {
			auto owned = std::make_unique<std::pmr::monotonic_buffer_resource>(
				local_buffer, local_buffer_size, std::pmr::null_memory_resource());
			LogResourceHandle handle;
			handle.resource = owned.get();
			handle.owned_resource = std::move(owned);
			return handle;
		}
	}

} // namespace tl2::internal