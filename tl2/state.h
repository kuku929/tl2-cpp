#pragma once
#include "log.h"
#include <exception>

namespace tl2 {
	class invalid_access : public std::exception {
	public:
		const char* what() const noexcept override {
			return "Accessing TVar outside transaction can lead to undefined behaviour.";
		}
	};
} // tl2


namespace tl2::internal {
	using namespace tl2::internal;

	class TMan {
	public:
		enum class STATE
		{
			ABORT,
			RUNNING,
			ZOMBIE,
		};

		void start_transaction() {
			log.clear();
			context.state = STATE::RUNNING;
		}

		void assert_in_transaction() {
			if(context.state != STATE::RUNNING)	{
				throw tl2::invalid_access();
			}
		}
	private:
		class Context {
			public:	
				STATE state;
				void clear() {
					state = STATE::ZOMBIE;
				}
		} context;
	} inline static thread_local manager;
} // tl2::internal
