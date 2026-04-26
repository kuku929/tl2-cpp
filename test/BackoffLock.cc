#include "tl2/tl2.h"
#include <thread>
#include <random>
#include <gtest/gtest.h>

using namespace tl2;

class BackoffLockSTM {
public:
    BackoffLockSTM(int min_delay = 1, int max_delay = 256)
        : state(false), min_delay(min_delay), max_delay(max_delay) {}

    void lock() {
        int limit = min_delay;
        std::mt19937 rng(std::random_device{}());

        while (true) {
            bool acquired = false;

            // Try to acquire via STM
            atomically([&]() {
                if (!(bool)state) {
                    state = true;
                    acquired = true;
                }
            });

            if (acquired) return;

            // Backoff outside transaction
            int delay = rng() % (limit + 1);
            for (int i = 0; i < delay; i++) {
                std::this_thread::yield();
            }

            limit = std::min(max_delay, limit * 2);
        }
    }

    void unlock() {
        atomically([&]() {
            state = false;
        });
    }

private:
    TVar<bool> state;
    int min_delay;
    int max_delay;
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
    t1.join(); t2.join(); t3.join();

    EXPECT_FALSE(violation);
}

