#include "profile.h"
#include "tl2/tl2.h"
#include <iostream>
#include <thread>
using namespace tl2;

uint run(const uint niters) {
  TVar<uint> counter(0);
  std::thread t1([&]() {
    for (uint _ = 0; _ < niters; ++_) {
      atomically([&]() {
        uint val = static_cast<uint>(counter);
        counter = val + 1;
      });
    }
  });
  std::thread t2([&]() {
    for (uint _ = 0; _ < niters; ++_) {
      atomically([&]() {
        uint val = static_cast<uint>(counter);
        counter = val + 1;
      });
    }
  });
  t1.join();
  t2.join();
  uint ans = 0;
  {
    atomically([&]() { ans = static_cast<uint>(counter); });
  }
  return ans;
}

int main() {
  for (uint i = 1; i < 100'000; i += 10'000) {
    run(i);
  }
  return 0;
}
