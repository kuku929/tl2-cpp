#include "tl2/tl2.h"
#include <vector>
#include <thread>
#include <gtest/gtest.h>

using namespace tl2;

class ALock {
public:
    explicit ALock(int capacity)
        : capacity(capacity),
          flags(capacity),
          tail(0) {

        // initialize flags
        for (int i = 0; i < capacity; i++) {
            flags[i] = (i == 0); // only slot 0 is true
        }
    }

    void lock() {
        int slot;

        // get slot atomically via STM
        atomically([&]() {
            int t = (int)tail;
            slot = t % capacity;
            tail = t + 1;
        });

        my_slot = slot;

        // spin until my flag is true
        while (true) {
            bool ready;

            atomically([&]() {
                ready = (bool)flags[slot];
            });

            if (ready) break;

            std::this_thread::yield(); // avoid busy burning
        }
    }

    void unlock() {
        int slot = my_slot;

        atomically([&]() {
            flags[slot] = false;
            flags[(slot + 1) % capacity] = true;
        });
    }

private:
    int capacity;
    std::vector<TVar<bool>> flags;
    TVar<int> tail;

    static thread_local int my_slot;
};

// thread-local storage
thread_local int ALock::my_slot = -1;

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