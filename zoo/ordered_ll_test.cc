#include "zoo/ordered_ll.h"

#include <gtest/gtest.h>
#include <thread>

namespace zoo {

TEST(OrderedLinkedListTests, AddContainsRemove) {
  OrderedLinkedList<int, std::hash<int>> list;

  EXPECT_FALSE(list.contains(10));

  EXPECT_TRUE(list.add(30));
  EXPECT_TRUE(list.add(10));
  EXPECT_TRUE(list.add(20));

  EXPECT_TRUE(list.contains(10));
  EXPECT_TRUE(list.contains(20));
  EXPECT_TRUE(list.contains(30));
  EXPECT_FALSE(list.contains(40));

  EXPECT_FALSE(list.add(20));

  EXPECT_TRUE(list.remove(10));
  EXPECT_FALSE(list.contains(10));
  EXPECT_TRUE(list.contains(20));
  EXPECT_TRUE(list.contains(30));

  EXPECT_FALSE(list.remove(10));
}

TEST(OrderedLinkedListTests, MultiThreadedAddRemove) {
  OrderedLinkedList<int, std::hash<int>> list;
  constexpr int kPerThread = 500;

  std::thread t1([&]() {
    for (int i = 0; i < kPerThread; ++i) {
      list.add(i);
    }
  });
  std::thread t2([&]() {
    for (int i = 0; i < kPerThread; ++i) {
      list.add(i + kPerThread);
    }
  });

  t1.join();
  t2.join();

  for (int i = 0; i < 2 * kPerThread; ++i) {
    EXPECT_TRUE(list.contains(i));
  }

  std::thread t3([&]() {
    for (int i = 0; i < kPerThread; ++i) {
      list.remove(i);
    }
  });
  std::thread t4([&]() {
    for (int i = 0; i < kPerThread; ++i) {
      list.remove(i + kPerThread);
    }
  });

  t3.join();
  t4.join();

  for (int i = 0; i < 2 * kPerThread; ++i) {
    EXPECT_FALSE(list.contains(i));
  }
}

} // namespace zoo
