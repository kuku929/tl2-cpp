#pragma once
#include "log.h"
#include "types.h"
#include <exception>

namespace tl2 {
class invalid_access : public std::exception {
public:
  const char *what() const noexcept override {
    return "Accessing TVar outside transaction can lead to undefined "
           "behaviour.";
  }
};
} // namespace tl2

namespace tl2::internal {
using namespace tl2::internal;

class TMan {
public:
  enum class STATE {
    ABORT,
    RUNNING,
    ZOMBIE,
  };

  void reset() {
    /*
    This does not free the heap allocated during the transaction
    Should be called when transaction succeeds.
    */
    context.state = STATE::ZOMBIE;
    log.deallocate_resources();
  }

  void start_transaction() {
    log.clear();
    context.state = STATE::RUNNING;
    context.rv = global_clock.get_version();
  }

  inline version_t read_version() { return context.rv; }

  void assert_in_transaction() {
    if (context.state != STATE::RUNNING) {
      log.clear();
      context.clear();
      throw tl2::invalid_access();
    }
  }

private:
  class Context {
  public:
    STATE state;
    version_t rv;
    void clear() {
      state = STATE::ZOMBIE;
      rv = static_cast<version_t>(0);
    }
  } context;
} inline static thread_local manager;
} // namespace tl2::internal
