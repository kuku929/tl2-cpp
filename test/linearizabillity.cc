/*
TL2 guarantees that transactions appear atomic
to other transactions. We check for this guarantee
in this test.
*/

#include "stm.h"
#include "tl2/tl2.h"
#include <atomic>
#include <gtest/gtest.h>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

using namespace tl2;

TEST(SimpleTests, Linearizability) {
  for (int _ = 0; _ < 100; ++_) {
    TVar<int> a(1), b(1);
    std::thread t1([&]() {
      try {
        atomically([&]() {
          a = 2;
          b = 2;
        });
      } catch (std::exception &ex) {
        std::cout << "weird " << ex.what() << std::endl;
      }
    });
    int val_a, val_b;
    std::thread t2([&]() {
      try {
        atomically([&]() {
          val_a = static_cast<int>(a);
          val_b = static_cast<int>(b);
        });
      } catch (std::exception &ex) {
        std::cout << "weird " << ex.what() << std::endl;
      }
    });
    t1.join();
    t2.join();
    std::cout << val_a << ' ' << val_b << std::endl;
    ASSERT_TRUE((val_a == 1 && val_b == 1) || (val_a == 2 && val_b == 2));
  }
}