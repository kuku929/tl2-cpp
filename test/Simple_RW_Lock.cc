#include "tl2/tl2.h"
#include "stm.h"
#include <thread>
#include <gtest/gtest.h>

using namespace tl2;

class RWLockSTM {
public:
    RWLockSTM() : readers(0), writer(false) {}

    void read_lock() {
        while (true) {
            bool acquired = false;

            atomically([&]() {
                if (!(bool)writer) {
                    readers = (int)readers + 1;
                    acquired = true;
                }
            });

            if (acquired) return;
            std::this_thread::yield();
        }
    }

    void read_unlock() {
        atomically([&]() {
            readers = (int)readers - 1;
        });
    }

    void write_lock() {
        while (true) {
            bool acquired = false;

            atomically([&]() {
                if (!(bool)writer && (int)readers == 0) {
                    writer = true;
                    acquired = true;
                }
            });

            if (acquired) return;
            std::this_thread::yield();
        }
    }

    void write_unlock() {
        atomically([&]() {
            writer = false;
        });
    }

private:
    TVar<int> readers;
    TVar<bool> writer;
};

//test 1: writer exclusive
TEST(STM, RWLockWriterExclusive) {
    RWLockSTM lock;
    int counter = 0;
    const int N = 5000;

    auto writer = [&]() {
        for (int i = 0; i < N; i++) {
            lock.write_lock();
            counter++;
            lock.write_unlock();
        }
    };

    std::thread t1(writer), t2(writer);
    t1.join();
    t2.join();

    EXPECT_EQ(counter, 2 * N);
}

//test 2: mutual exclusion check
TEST(STM, RWLockMutualExclusionViolation) {
    RWLockSTM lock;
    int inside = 0;
    bool violation = false;
    const int N = 3000;

    auto writer = [&]() {
        for (int i = 0; i < N; i++) {
            lock.write_lock();

            inside++;
            if (inside > 1) violation = true;

            inside--;
            lock.write_unlock();
        }
    };

    std::thread t1(writer), t2(writer), t3(writer);
    t1.join(); t2.join(); t3.join();

    EXPECT_FALSE(violation);
}

//test 3: checking if multiple readers can co exist
TEST(STM, RWLockMultipleReaders) {
    RWLockSTM lock;
    int shared = 42;

    auto reader = [&]() {
        for (int i = 0; i < 2000; i++) {
            lock.read_lock();
            EXPECT_EQ(shared, 42);
            lock.read_unlock();
        }
    };

    std::thread t1(reader), t2(reader), t3(reader);
    t1.join(); t2.join(); t3.join();
}

//test 4: no read happens during write
TEST(STM, RWLockNoReadDuringWrite) {
    RWLockSTM lock;
    int shared = 0;
    bool violation = false;

    std::thread writer([&]() {
        for (int i = 0; i < 2000; i++) {
            lock.write_lock();
            shared++;
            std::this_thread::sleep_for(std::chrono::microseconds(5));
            lock.write_unlock();
        }
    });

    std::thread reader([&]() {
        for (int i = 0; i < 2000; i++) {
            lock.read_lock();
            int val = shared;
            if (val < 0) violation = true;
            lock.read_unlock();
        }
    });

    writer.join();
    reader.join();

    EXPECT_FALSE(violation);
}