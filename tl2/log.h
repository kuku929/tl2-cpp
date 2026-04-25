#pragma once
#include "hash_table.h"
#include "read_set.h"
#include "types.h"
#include "write_set.h"
#include <cassert>
#include <optional>
#include <array>
#include <memory_resource>
#include <cstring>
#include "memory.h"

namespace tl2::internal {
	using namespace tl2::internal;
	template<typename WriteSetT, typename ReadSetT>
	class Log {
	public:
		Log()
		    : r(), w(), buf{} {
			res = make_transaction_resource(buf.data(), buf.size());
			buf.fill(std::byte{0});
		}
		
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
			if(r.contains(op))
				r.modify(op);
			else r.insert(op);
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
			void* storage = res->allocate(nbytes, alignof(T));
			std::memcpy(storage, reinterpret_cast<const void*>(&val), nbytes);
			const T* copied_ptr = reinterpret_cast<const T*>(storage);
			const WriteOp &op = { addr, copied_ptr };
			w.insert(op);
		}

		void clear() {
			r.clear();
			w.clear();
			// reset memory resource so allocations are reclaimed for next transaction
			res.reset();
			buf.fill(std::byte{0});
			res = make_transaction_resource(buf.data(), buf.size());
		}

		WriteSetT& writes() {
			return w;
		}

		ReadSetT& reads() {
			return r;
		}

	private:
		ReadSetT r;
		WriteSetT w;
		// per-transaction contiguous buffer and memory resource for write copies
		static constexpr std::size_t kBufSize = 4 * 1024;
		std::array<std::byte, kBufSize> buf;
		std::unique_ptr<std::pmr::monotonic_buffer_resource> res;
		// unique_ptr because it lets you reset() and make_unique() a new resource 
		// when clearing a transaction (what clear() does).
	};
	inline static thread_local Log<WriteOrderedSet, ReadOrderedSet> log;
} // tl2::internal