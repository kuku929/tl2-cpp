#include <gtest/gtest.h>
#include <array>
#include <thread>
#include "tl2/tl2.h"

using namespace tl2;

struct PetersonLock {
 public:
  PetersonLock() : flag({0, 0}), victim(0) { ; }
  void lock(uint id) {
    atomically([&]() {
      flag[id] = 1u;
      victim = id;
      while (static_cast<uint>(flag[1 - id]) == 1 and
             static_cast<uint>(victim) == id) {}
    });
  };
  void unlock(uint id) {
    atomically([&]() { flag[id] = 0; });
  };

 private:
  std::array<TVar<uint>, 2> flag;
  TVar<uint> victim;
};

struct BakeryLock {
 public:
  explicit BakeryLock(size_t n) 
    : choosing(n,TVar<bool>(false)), number(n,TVar<uint>(0)) {}

  void lock(uint id) {
    // Step 1: choosing[id] = true
    atomically([&]() { choosing[id] = true; });

    // Step 2: number[id] = 1 + max(number[])
    atomically([&]() {
      uint max_val = 0;
      for (size_t i = 0; i < number.size(); ++i) {
        uint val = static_cast<uint>(number[i]);
        if (val > max_val)
          max_val = val;
      }
      number[id] = max_val + 1;
    });

    // Step 3: choosing[id] = false
    atomically([&]() { choosing[id] = false; });

    // Step 4: wait
    for (size_t j = 0; j < number.size(); ++j) {
      if (j == id)
        continue;

      // wait while choosing[j]
      while (true) {
        bool cj;
        atomically([&]() { cj = static_cast<bool>(choosing[j]); });
        if (!cj)
          break;
        std::this_thread::yield();
      }

      // wait while number[j] != 0 and (number[j], j) < (number[id], id)
      while (true) {
        uint nj, ni;
        atomically([&]() {
          nj = static_cast<uint>(number[j]);
          ni = static_cast<uint>(number[id]);
        });

        if (nj == 0)
          break;

        if (nj < ni || (nj == ni && j < id)) {
          std::this_thread::yield();
          continue;
        }
        break;
      }
    }
  }

  void unlock(uint id) {
    atomically([&]() { number[id] = 0; });
  }

 private:
  std::vector<TVar<bool>> choosing;
  std::vector<TVar<uint>> number;
};

TEST(SimpleTests, BakeryLockBasic) {
  BakeryLock l(2);

  auto run = [&](uint niters) {
    uint counter = 0;

    std::thread t1([&]() {
      for (uint i = 0; i < niters; ++i) {
        l.lock(0);
        counter++;
        l.unlock(0);
      }
    });

    std::thread t2([&]() {
      for (uint i = 0; i < niters; ++i) {
        l.lock(1);
        counter++;
        l.unlock(1);
      }
    });

    t1.join();
    t2.join();

    return counter;
  };

  for (uint i = 1; i <= 50000; i += 10000) {
    EXPECT_EQ(run(i), 2 * i);
  }
}

TEST(SimpleTests, BakeryLockMutualExclusion) {
  BakeryLock l(2);
  int inside = 0;
  bool violation = false;

  std::thread t1([&]() {
    for (int i = 0; i < 5000; ++i) {
      l.lock(0);
      inside++;
      if (inside > 1)
        violation = true;
      inside--;
      l.unlock(0);
    }
  });

  std::thread t2([&]() {
    for (int i = 0; i < 5000; ++i) {
      l.lock(1);
      inside++;
      if (inside > 1)
        violation = true;
      inside--;
      l.unlock(1);
    }
  });

  t1.join();
  t2.join();

  EXPECT_FALSE(violation);
}

TEST(SimpleTests, BakeryLockThreeThreads) {
    BakeryLock l(3);
    uint counter = 0;
    const uint N = 4000;

    auto worker = [&](uint id) {
        for (uint i = 0; i < N; ++i) {
            l.lock(id);
            counter++;
            l.unlock(id);
        }
    };

    std::thread t1(worker, 0);
    std::thread t2(worker, 1);
    std::thread t3(worker, 2);

    t1.join();
    t2.join();
    t3.join();

    EXPECT_EQ(counter, 3 * N);
}

//may expose livelock/slowdown
TEST(SimpleTests, BakeryLockStress) {
  BakeryLock l(2);
  uint counter = 0;
  const uint N = 20000;

  std::thread t1([&]() {
    for (uint i = 0; i < N; ++i) {
      l.lock(0);
      counter++;
      l.unlock(0);
    }
  });

  std::thread t2([&]() {
    for (uint i = 0; i < N; ++i) {
      l.lock(1);
      counter++;
      l.unlock(1);
    }
  });

  t1.join();
  t2.join();

  EXPECT_EQ(counter, 2 * N);
}
