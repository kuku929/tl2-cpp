#include "zoo/hash_map.h"

#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <thread>

namespace zoo {

struct ZeroHash {
  std::size_t operator()(int) const { return 0u; }
};

TEST(HashMapTests, GetSetContains) {
  HashMap<int, int, std::hash<int>> map(std::hash<int>{});

  tl2::atomically([&]() {
    EXPECT_FALSE(map.contains(1));

    int default_val = map.get(1);
    EXPECT_EQ(default_val, 0);
    EXPECT_TRUE(map.contains(1));

    map.set(1, 42);
    EXPECT_EQ(map.get(1), 42);
    EXPECT_TRUE(map.contains(1));
  });
}

TEST(HashMapTests, HandlesCollisions) {
  HashMap<int, int, ZeroHash> map(ZeroHash{});

  tl2::atomically([&]() {
    map.set(1, 10);
    map.set(2, 20);

    EXPECT_TRUE(map.contains(1));
    EXPECT_TRUE(map.contains(2));
    EXPECT_EQ(map.get(1), 10);
    EXPECT_EQ(map.get(2), 20);

    map.set(1, 100);
    EXPECT_EQ(map.get(1), 100);
    EXPECT_EQ(map.get(2), 20);
  });
}

TEST(HashMapTests, MultiThreadedSetAndGet) {
  HashMap<int, int, std::hash<int>> map(std::hash<int>{});
  constexpr int kPerThread = 500;

  std::thread t1([&]() {
    for (int i = 0; i < kPerThread; ++i) {
      tl2::atomically([&]() { map.set(i, i * 10); });
    }
  });
  std::thread t2([&]() {
    for (int i = 0; i < kPerThread; ++i) {
      tl2::atomically([&]() { map.set(i + kPerThread, i * 10); });
    }
  });

  t1.join();
  t2.join();

  tl2::atomically([&]() {
    for (int i = 0; i < 2 * kPerThread; ++i) {
      EXPECT_TRUE(map.contains(i));
    }
  });
}

} // namespace zoo
