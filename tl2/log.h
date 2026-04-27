#pragma once
#include "hash_table.h"
#include "read_set.h"
#include "types.h"
#include "write_set.h"
#include <cassert>
#include <optional>
#include <memory>
#include <memory_resource>
#include <cstring>
#include "memory.h"

namespace tl2::internal {
	using namespace tl2::internal;
	template<typename WriteSetT, typename ReadSetT>
	class Log {
	public:
		// Default: use per-log monotonic resource backed by default upstream allocation
		Log()
			: r(), w() {
			owned_resource = std::make_unique<std::pmr::monotonic_buffer_resource>();
			resource = owned_resource.get();
		}

		// Use an externally provided memory resource (synchronized_pool_resource)
		explicit Log(std::pmr::memory_resource* external_resource)
			: r(), w(), resource(external_resource) {}
		
		template<typename T>
		std::optional<T> value_at(const T* addr) const {
			// check in write set for this address
			const std::optional<addr_t> entry = w.find_opt(reinterpret_cast<addr_t>(addr));
			if(entry.has_value()) {
				return *reinterpret_cast<const T*>(entry.value());
			} 
			return std::nullopt;
		}

		template<typename T>
		void append_read(const T* addr) {
			const addr_t a = reinterpret_cast<addr_t>(addr);
			const ReadOp &op = { a, hashtbl[a].get_version() };
			r.update(op);
		}

		template<typename T>
		void append_write(const T* addr, const T& val) noexcept {
			const addr_t a = reinterpret_cast<addr_t>(addr);
			const auto entry = w.find_opt(a);
			if ( entry.has_value()) {
				// Address already in write-set: overwrite existing staged value.
				std::memcpy(reinterpret_cast<void*>(entry.value()), reinterpret_cast<const void*>(&val), sizeof(T));
				return;
				// w.modify not required
			}

			// First write for this address in this transaction: stage a copied value.
			const std::size_t nbytes = sizeof(T);
			void* storage = active_resource().allocate(nbytes, alignof(T));
			std::memcpy(storage, reinterpret_cast<const void*>(&val), nbytes);
			const T* copied_ptr = reinterpret_cast<const T*>(storage);
			const WriteOp &op = { addr, copied_ptr };
			w.update(op);
		}

		void clear() {
			r.clear();
			w.clear();
			// reset memory resource so allocations are reclaimed for next transaction
			// if we own the monotonic buffer, recreate it to reclaim
			// if we use an external resource, what should I do?????
			if (owned_resource) {
				owned_resource = std::make_unique<std::pmr::monotonic_buffer_resource>();
				resource = owned_resource.get();
			}
		}

		WriteSetT& writes() {
			return w;
		}

		ReadSetT& reads() {
			return r;
		}

	private:
		std::pmr::memory_resource& active_resource() {
			assert(resource != nullptr);
			return *resource;
		}

		ReadSetT r;
		WriteSetT w;
		std::pmr::memory_resource* resource{nullptr};
		std::unique_ptr<std::pmr::monotonic_buffer_resource> owned_resource;
	};
	inline static thread_local Log<WriteOrderedSet, ReadOrderedSet> log;
	// inline static thread_local Log<WriteOrderedSet, ReadOrderedSet> log(&internal::synchronized_pool_resource);
} // tl2::internal