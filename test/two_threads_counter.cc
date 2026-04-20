#include <iostream>
#include <thread>
#include "tl2/tl2.h"
#include <gtest/gtest.h>

uint run(const uint niters) {
    TVar counter(0);
    std::thread t1([&]() {
        STM::atomically([&]() {
            for(uint _ = 0; _ < niters; ++_) {
                uint val = counter.get();
                counter.set(val + 1);
            }
        });
    });
    std::thread t2([&]() {
        STM::atomically([&]() {
            for(uint _ = 0; _ < niters; ++_) {
                uint val = counter.get();
                counter.set(val + 1);
            }
        });
    });
    t1.join();
    t2.join();
    uint ans = 0; {
        STM::atomically([&]() {
            ans = counter.get();
        });
    }
    return ans;
}

TEST(SimpleTests, TwoThreadsCounter) {
    for(uint _ = 1; _ < 100'000; _ += 10'000) {
        EXPECT_EQ(run(_), 2 * _);
    }
}
