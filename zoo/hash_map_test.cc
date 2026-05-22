#include "zoo/hash_map.h"

#include <atomic>
#include <gtest/gtest.h>
#include <thread>

namespace zoo {

struct ZeroHash {
  std::size_t operator()(int) const { return 0u; }
};

TEST(HashMapTests, GetSetContains) {
  HashMap<int, int, std::hash<int>> map(std::hash<int>{});

  EXPECT_FALSE(map.contains(1));

  map.set(1, 42);
  EXPECT_EQ(map.get(1), 42);
  EXPECT_TRUE(map.contains(1));
}

TEST(HashMapTests, HandlesCollisions) {
  HashMap<int, int, ZeroHash> map(ZeroHash{});

  map.get(1);
  map.get(2);

  map.set(1, 10);
  map.set(2, 20);

  EXPECT_TRUE(map.contains(1));
  EXPECT_TRUE(map.contains(2));
  EXPECT_EQ(map.get(1), 10);
  EXPECT_EQ(map.get(2), 20);

  map.set(1, 100);
  EXPECT_EQ(map.get(1), 100);
  EXPECT_EQ(map.get(2), 20);
}

TEST(HashMapTests, MultiThreadedReads) {
  HashMap<int, int, std::hash<int>> map(std::hash<int>{});
  constexpr int kPerThread = 200;

  for (int i = 0; i < 2 * kPerThread; ++i) {
    map.get(i);
    map.set(i, i * 10);
  }

  std::atomic<int> failures{0};
  std::atomic<bool> start{false};

  std::thread t1([&]() {
    while (!start.load()) {
    }
    for (int i = 0; i < 2 * kPerThread; ++i) {
      if (!map.contains(i)) {
        ++failures;
      }
    }
  });

  std::thread t2([&]() {
    while (!start.load()) {
    }
    for (int i = 0; i < 2 * kPerThread; ++i) {
      if (map.get(i) != i * 10) {
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
