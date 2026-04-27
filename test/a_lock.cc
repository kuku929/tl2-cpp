#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace tl2;

class ALock {
public:
  explicit ALock(size_t cap)
      : capacity(cap), flags(cap, TVar<bool>(false)), tail(0) {
    // only slot 0 is true
    atomically([&]() { flags[0] = true; });
  }

  void lock() {
    size_t slot;

    // get slot atomically via STM
    atomically([&]() {
      size_t t = static_cast<size_t>(tail);
      slot = t % capacity;
      tail = t + 1;
    });

    my_slot = slot;

    // spin until my flag is true
    while (true) {
      bool ready;

      atomically([&]() { ready = static_cast<bool>(flags[slot]); });

      if (ready)
        break;

      std::this_thread::yield(); // avoid busy burning
    }
  }

  void unlock() {
    size_t slot = my_slot;

    atomically([&]() {
      flags[slot] = false;
      flags[(slot + 1) % capacity] = true;
    });
  }

private:
  size_t capacity;
  std::vector<TVar<bool>> flags;
  TVar<size_t> tail;

  static thread_local size_t my_slot;
};

// thread-local storage
thread_local size_t ALock::my_slot = 0;

TEST(ALockSTM, MutualExclusion) {
  ALock lock(8);
  int counter = 0;
  const int N = 5000;

  std::thread t1([&]() {
    for (int i = 0; i < N; i++) {
      lock.lock();
      counter++;
      lock.unlock();
    }
  });

  std::thread t2([&]() {
    for (int i = 0; i < N; i++) {
      lock.lock();
      counter++;
      lock.unlock();
    }
  });

  t1.join();
  t2.join();

  EXPECT_EQ(counter, 2 * N);
}