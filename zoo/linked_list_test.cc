#include "zoo/linked_list.h"

#include "tl2/tl2.h"
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

  tl2::atomically([&]() { list.update(5, 50); });
  EXPECT_FALSE(list.contains(5));
  EXPECT_TRUE(list.contains(50));

  auto updated = list.get(50);
  ASSERT_TRUE(updated.has_value());
  EXPECT_EQ(*updated, 50);
}

TEST(LinkedListTests, MultiThreadedAddRemove) {
  LinkedList<int> list;
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

  EXPECT_EQ(list.size(), static_cast<std::size_t>(2 * kPerThread));

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

  EXPECT_EQ(list.size(), 0u);
}

} // namespace zoo
