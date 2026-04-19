#include <iostream>
#include <thread>
#include "tl2/tl2.h"

int main(int argc, char *argv[]) {
    // TODO : test cases(a lot of them)
    TVar counter(0);
    std::thread t1([&]() {
        STM::atomically([&]() {
            for(int __i = 0; __i < 100'000; ++__i) {
                uint val = counter.get();
                counter.set(val + 1);
            }
        });
    });
    std::thread t2([&]() {
        STM::atomically([&]() {
            for(int __i = 0; __i < 100'000; ++__i) {
                uint val = counter.get();
                counter.set(val + 1);
            }
        });
    });
    t1.join();
    t2.join();
    STM::atomically([&]() {
        std::cout << counter.get() << std::endl;
    });
}
