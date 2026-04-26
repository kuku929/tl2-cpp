#include "tl2/tl2.h"
#include <gtest/gtest.h>
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
      std::this_thread::sleep_for(std::chrono::nanoseconds(50));
    }
  });
  std::thread t2([&]() {
    for (uint _ = 0; _ < niters; ++_) {
      atomically([&]() {
          uint val = static_cast<uint>(counter);
          counter = val + 1;
        });
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
  });
  std::cout<<"before joining\n";
  t1.join();
  t2.join();
  uint ans = 0;
  {
    atomically([&]() { ans = static_cast<uint>(counter); });
  }
  std::cout<<"after joining\n";
  std::cout<<ans<<" "<<niters<<"\n";
  return ans;
}

TEST(SimpleTests, TwoThreadsCounter) {
  for (uint i = 1; i < 100'0; i += 10'0) {
    EXPECT_EQ(run(i), 2 * i);
  }
}
