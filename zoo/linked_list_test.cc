#include "zoo/linked_list.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>

namespace zoo {

TEST(LinkedListTests, AddContainsRemoveAndSize) {
  LinkedList<int> list;

  EXPECT_EQ(list.size(), 0u);
  EXPECT_FALSE(list.contains(10));

  list.add(10);
  list.add(20);
  list.add(30);

  EXPECT_EQ(list.size(), 3u);
  EXPECT_TRUE(list.contains(10));
  EXPECT_TRUE(list.contains(20));
  EXPECT_TRUE(list.contains(30));
  EXPECT_FALSE(list.contains(40));

  EXPECT_TRUE(list.remove(20));
  EXPECT_FALSE(list.contains(20));
  EXPECT_EQ(list.size(), 2u);

  EXPECT_FALSE(list.remove(40));
  EXPECT_EQ(list.size(), 2u);

  EXPECT_TRUE(list.remove(10));
  EXPECT_EQ(list.size(), 1u);
  EXPECT_TRUE(list.remove(30));
  EXPECT_EQ(list.size(), 0u);
}

TEST(LinkedListTests, GetAndUpdate) {
  LinkedList<int> list;

  EXPECT_FALSE(list.get(5).has_value());

  list.add(5);
  list.add(7);

  auto val = list.get(5);
  ASSERT_TRUE(val.has_value());
  EXPECT_EQ(*val, 5);

  list.update(5, 50);
  EXPECT_FALSE(list.contains(5));
  EXPECT_TRUE(list.contains(50));

  auto updated = list.get(50);
  ASSERT_TRUE(updated.has_value());
  EXPECT_EQ(*updated, 50);
}

TEST(LinkedListTests, MultiThreadedReads) {
  LinkedList<int> list;
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
      auto val = list.get(i);
      if (!val.has_value() || *val != i) {
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
