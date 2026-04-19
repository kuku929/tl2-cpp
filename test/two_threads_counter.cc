#include <iostream>
#include <thread>
#include "tl2/tl2.h"
#include <gtest/gtest.h>

uint run(const uint niters) {
    TVar counter(0);
    std::thread t1([&]() {
        STM::atomically([&]() {
            for(uint __i = 0; __i < niters; ++__i) {
                uint val = counter.get();
                counter.set(val + 1);
            }
        });
    });
    std::thread t2([&]() {
        STM::atomically([&]() {
            for(uint __i = 0; __i < niters; ++__i) {
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
    for(uint niters = 1; niters < 100'000; niters += 1000) {
        EXPECT_EQ(run(niters), 2 * niters);
    }
}
