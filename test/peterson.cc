#include "stm.h"
#include <thread>
#include <array>
#include "tl2/tl2.h"
#include <gtest/gtest.h>

struct PetersonLock {
public:
    PetersonLock() : flag({0, 0}), victim(0) {;}
    void lock(uint id) {
        STM::atomically([&]() {
            flag[id].set(1);
            victim.set(id);
            while(flag[1 - id].get() and victim.get() == id) {}
        });
    };
    void unlock(uint id) {
        STM::atomically([&]() {
            flag[id].set(0);
        });
    };
private:
    std::array<TVar, 2> flag;
    TVar victim;    
};

uint run(const uint niters) {
    PetersonLock l;
    uint counter = 0;
    std::thread t1([&]() {
        for(uint _ = 0; _ < niters; ++_) {
            l.lock(0);
            counter++;
            l.unlock(0);
        }
    });
    std::thread t2([&]() {
        for(uint _ = 0; _ < niters; ++_) {
            l.lock(1);
            counter++;
            l.unlock(1);
        }
    });
    t1.join();
    t2.join();
    return counter;
}

TEST(SimpleTests, PetersonLock) {
    for(uint _ = 1; _ < 100'000; _ += 10'000) {
        EXPECT_EQ(run(_), 2 * _);
    }
}
