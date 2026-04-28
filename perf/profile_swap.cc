#include "tl2/tl2.h"
#include <chrono>
#include <iostream>
#include <thread>

int main() {
    /*
    This test revealed that std::function can be a real bottleneck
    when objects are destroyed and created many many times a second.
    Using a faster implementation function that is around 1.1x faster.
    -O3 leads to a huge improvement(1.6 -> 0.16)
    */
    tl2::TVar<int> a(1), b(2);
    auto start = std::chrono::high_resolution_clock::now();
    std::thread t1([&a, &b]() {
        for(int _ = 0; _ < 1'000'000; ++_) {
            tl2::atomically([&a, &b]() {
                int a_val = static_cast<int>(a);
                int b_val = static_cast<int>(b);
                a = b_val;
                b = a_val;
            });
        }
    });
    std::thread t2([&a, &b]() {
        for (uint _ = 0; _ < 1'000'000; ++_) {
            tl2::atomically([&a, &b]() {
                int a_val = static_cast<int>(a);
                int b_val = static_cast<int>(b);
                a = b_val;
                b = a_val;
            });
        }
    });
    t1.join(); t2.join();
    auto end = std::chrono::high_resolution_clock::now();
    double seconds = std::chrono::duration<double>(end - start).count();
    std::cerr << seconds << std::endl;
    return 0;
}