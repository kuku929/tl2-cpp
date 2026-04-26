#include "stm.h"
#include "tl2/tl2.h"
#include <thread>
#include <gtest/gtest.h>

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

//test 1: testing mutual exclusion for writers
TEST(STM, RWLockWriterExclusive) {
    RWLockSTM lock;
    int shared = 0;
    bool violation = false;

    auto writer = [&]() {
        for (int i = 0; i < 5000; i++) {
            lock.write_lock();

            int temp = shared;
            temp++;
            shared = temp;

            lock.write_unlock();
        }
    };

    std::thread t1(writer), t2(writer);
    t1.join(); t2.join();

    EXPECT_EQ(shared, 10000);
}

//test 2: readers can read concurrently
TEST(STM, RWLockMultipleReaders) {
    RWLockSTM lock;
    int shared = 42;

    auto reader = [&]() {
        for (int i = 0; i < 1000; i++) {
            lock.read_lock();
            EXPECT_EQ(shared, 42);
            lock.read_unlock();
        }
    };

    std::thread t1(reader), t2(reader), t3(reader);
    t1.join(); t2.join(); t3.join();
}

//test 3: no read during write
TEST(STM, RWLockNoReadDuringWrite) {
    RWLockSTM lock;
    int shared = 0;
    bool violation = false;

    std::thread writer([&]() {
        for (int i = 0; i < 1000; i++) {
            lock.write_lock();
            shared++;
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            lock.write_unlock();
        }
    });

    std::thread reader([&]() {
        for (int i = 0; i < 1000; i++) {
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