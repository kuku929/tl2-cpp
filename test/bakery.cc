#include "stm.h"
#include <thread>
#include <array>
#include "tl2/tl2.h"
#include <gtest/gtest.h>

struct BakeryLock {
public:
    BakeryLock(int nthreads) : tickets(std::vector<TVar>(nthreads, 0u)) {;}

    bool conflict(uint id, uint ticket) {
        for(size_t id1 = 0; id1 < tickets.size(); ++id) {
            const auto &t = tickets[id1];
            if(t.get() < ticket or (t.get() == ticket and id1 < id)) {
                return false;
            }
        }
        return true;
    };

    void lock(uint id) {
        STM::atomically([&]() {
            uint max = 0;
            for(const auto t : tickets) {
                max = std::max(t.get(), max);
            }
            tickets[id].set(max + 1);

        });
    };
    void unlock(uint id) {
        STM::atomically([&]() {
            flag[id].set(0);
        });
    };
private:
    std::vector<TVar> tickets;
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
