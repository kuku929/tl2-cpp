#include "tl2/tl2.h"
#include <iostream>
#include <thread>
using namespace tl2;

uint run(const uint niters) {
  TVar<uint> counter(0);

  std::thread t1([&]() {
    try {
        for (uint _ = 0; _ < niters; ++_) {
        atomically([&]() {
            uint val = (uint)counter;
            counter = val + 1;
        });
        std::this_thread::yield();
        }
    } catch (...) {
        std::cout << "Exception in t1\n";
    }
    });

    std::thread t2([&]() {
        try {
            for (uint _ = 0; _ < niters; ++_) {
            atomically([&]() {
                uint val = (uint)counter;
                counter = val + 1;
            });
            std::this_thread::yield();
            }
        } catch (...) {
            std::cout << "Exception in t1\n";
        }
        });

  t1.join();
  t2.join();

  uint ans = 0;
  atomically([&]() { ans = (uint)counter; });
  return ans;
}

int main() {
  for (uint i = 1; i <= 1000; i += 100) {
    std::cout << "Running i=" << i << std::endl;

    // uint res = run(i);

    // std::cout << "Result = " << res
    //           << " Expected = " << 2 * i << std::endl;

    // if (res != 2 * i) {
    //   std::cout << "❌ FAILED\n";
    //   return 0;
    // } else {
    //   std::cout << "✅ OK\n";
    // }
    TVar<int> x(2);
    x=4;
  }
}