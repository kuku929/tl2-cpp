#include "zoo/ordered_ll.h"

#include <atomic>
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

TEST(OrderedLinkedListTests, MultiThreadedReads) {
  OrderedLinkedList<int, std::hash<int>> list;
  constexpr int kPerThread = 200;

  for (int i = 0; i < 2 * kPerThread; ++i) {
    list.add(i);
  }

  std::atomic<int> failures{0};
  std::atomic<bool> start{false};

  std::thread t1([&]() {
    while (!start.load()) {
    }
    for (int i = 0; i < 2 * kPerThread; ++i) {
      if (!list.contains(i)) {
        ++failures;
      }
    }
  });

  std::thread t2([&]() {
    while (!start.load()) {
    }
    for (int i = 0; i < 2 * kPerThread; ++i) {
      if (!list.contains(i)) {
        ++failures;
      }
    }
  });

  start.store(true);
  t1.join();
  t2.join();

  EXPECT_EQ(failures.load(), 0);
}

} // namespace zoo
