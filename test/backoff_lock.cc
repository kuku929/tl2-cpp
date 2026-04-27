#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <random>
#include <thread>

using namespace tl2;

class BackoffLockSTM {
public:
  BackoffLockSTM(size_t mn_delay = 1, size_t mx_delay = 256)
      : state(false), min_delay(mn_delay), max_delay(mx_delay) {}

  void lock() {
    size_t limit = min_delay;
    std::mt19937 rng(std::random_device{}());

    while (true) {
      bool acquired = false;

      // Try to acquire via STM
      atomically([&]() {
        if (!static_cast<bool>(state)) {
          state = true;
          acquired = true;
        }
      });

      if (acquired)
        return;

      // Backoff outside transaction
      size_t delay = rng() % (limit + 1);
      for (size_t i = 0; i < delay; i++) {
        std::this_thread::yield();
      }

      limit = std::min(max_delay, limit * 2);
    }
  }

  void unlock() {
    atomically([&]() { state = false; });
  }

private:
  TVar<bool> state;
  size_t min_delay;
  size_t max_delay;
};

TEST(STM, BackoffLockCorrectness) {
  BackoffLockSTM lock;
  int counter = 0;
  const int N = 10000;

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

  std::thread t3([&]() {
    for (int i = 0; i < N; i++) {
      lock.lock();
      counter++;
      lock.unlock();
    }
  });

  t1.join();
  t2.join();
  t3.join();

  EXPECT_EQ(counter, 3 * N);
}

TEST(STM, BackoffLockMutualExclusion) {
  BackoffLockSTM lock;
  int inside = 0;
  bool violation = false;
  const int N = 5000;

  auto worker = [&]() {
    for (int i = 0; i < N; i++) {
      lock.lock();

      inside++;
      if (inside > 1) {
        violation = true;
      }

      inside--;
      lock.unlock();
    }
  };

  std::thread t1(worker), t2(worker), t3(worker);
  t1.join();
  t2.join();
  t3.join();

  EXPECT_FALSE(violation);
}
