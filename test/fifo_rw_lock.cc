#include "stm.h"
#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <thread>
using namespace tl2;
/*
CANNOT BE IMPLEMENTED WITH STM
*/
class RWLockSTM {
public:
  RWLockSTM() : readers(0), writer(false) {}

  void read_lock() {
    atomically([&]() {
      if (!static_cast<bool>(writer)) {
        readers = static_cast<int>(readers) + 1;
      }
    });
  }

  void read_unlock() {
    atomically([&]() { readers = static_cast<int>(readers) - 1; });
  }

  void write_lock() {
    atomically([&]() {
      if (!static_cast<bool>(writer) && static_cast<int>(readers) == 0) {
        writer = true;
      }
    });
  }

  void write_unlock() {
    atomically([&]() { writer = false; });
  }

private:
  TVar<int> readers;
  TVar<bool> writer;
};

// test 1: testing mutual exclusion for writers
TEST(STM, RWLockWriterExclusive) {
  RWLockSTM lock;
  int shared = 0;
  bool violation = false;

  auto writer = [&]() {
    for (int i = 0; i < 50; i++) {
      lock.write_lock();

      shared++;

      lock.write_unlock();
    }
  };

  std::thread t1(writer), t2(writer);
  t1.join();
  t2.join();

  EXPECT_EQ(shared, 100);
}

// test 2: readers can read concurrently
TEST(STM, RWLockMultipleReaders) {
  RWLockSTM lock;
  int shared = 42;

  auto reader = [&]() {
    for (int i = 0; i < 10; i++) {
      lock.read_lock();
      EXPECT_EQ(shared, 42);
      lock.read_unlock();
    }
  };

  std::thread t1(reader), t2(reader), t3(reader);
  t1.join();
  t2.join();
  t3.join();
}

// test 3: no read during write
TEST(STM, RWLockNoReadDuringWrite) {
  RWLockSTM lock;
  int shared = 0;
  bool violation = false;

  std::thread writer([&]() {
    for (int i = 0; i < 100; i++) {
      lock.write_lock();
      shared++;
      std::this_thread::sleep_for(std::chrono::microseconds(10));
      lock.write_unlock();
    }
  });

  std::thread reader([&]() {
    for (int i = 0; i < 100; i++) {
      lock.read_lock();
      int val = shared;
      if (val < 0)
        violation = true;
      lock.read_unlock();
    }
  });

  writer.join();
  reader.join();

  EXPECT_FALSE(violation);
}