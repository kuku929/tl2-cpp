#pragma once
#include "hash_table.h"
#include "read_set.h"
#include "types.h"
#include "write_set.h"
#include <cassert>
#include <optional>
#include "memory.h"

namespace tl2::internal {
	using namespace tl2::internal;
	template<typename WriteSetT, typename ReadSetT>
	class Log {
	public:
		Log() : r(),
				w() {;}
		
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
		};

		template<typename T>
		void append_write(const T* addr, const T& val) noexcept {
			const WriteOp &op = { addr, &val };
			if(w.contains(op))
				w.modify(op);
			else w.insert(op);
		};

		void clear() {
			r.clear();
			w.clear();
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
	};
	inline static thread_local Log<WriteOrderedSet, ReadOrderedSet> log;
} // tl2::internal