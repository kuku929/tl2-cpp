#include "tl2/tl2.h"
#include <gtest/gtest.h>
#include <mutex> //used for tests
#include <optional>
#include <set>
#include <thread>
#include <vector>

using namespace tl2;

template <typename T> struct BoundedQueue {
public:
  BoundedQueue(size_t cap)
      : _capacity(cap), items(cap, TVar<T>(T())), head(0), tail(0) {}

  bool try_enq(const T &x) {
    bool success = false;

    tl2::atomically([&]() {
      success = enq_tx(x);
    });

    return success;
  }

  // returns false if the queue is empty, otherwise returns true and sets out
  bool try_deq(T& out) {
    bool success = false;

    tl2::atomically([&]() {
      auto res = deq_tx();
      if (res.first) {
        out = res.second;
        success = true;
      }
    });

    return success;
  }

  size_t capacity() const { return _capacity; }

  size_t size() const {
    size_t size;
    tl2::atomically([&]() {
      size = static_cast<size_t>(tail) - static_cast<size_t>(head);
    });
    return size;
  }

  // Batch enqueue/dequeue in a single transaction for better performance
  // Usage: atomically([&]() { q.enq_tx(x); q.deq_tx(); });
  bool enq_tx(const T& x) {
    size_t h = static_cast<size_t>(head);
    size_t t = static_cast<size_t>(tail);

    if (t - h == _capacity) {
      return false;
    }

    items[t % _capacity] = x;
    tail = t + 1;
    return true;
  }

  std::pair<bool, T> deq_tx() {
    size_t h = static_cast<size_t>(head);
    size_t t = static_cast<size_t>(tail);

    if (t == h) {
      return {false, T()};
    }

    auto result = static_cast<T>(items[h % _capacity]);
    head = h + 1;
    return {true, result};
  }

private:
  const size_t _capacity;
  std::vector<TVar<T>> items;
  TVar<size_t> head, tail;
};

// testing the basic functionality of the queue
// enqueue two values and then dequeue them
TEST(BoundedQueue, EnqueueDequeueSingle) {
  BoundedQueue<int> q(5);

  EXPECT_TRUE(q.try_enq(10));
  EXPECT_TRUE(q.try_enq(20));

  int x;
  ASSERT_TRUE(q.try_deq(x));
  EXPECT_EQ(x, 10);

  ASSERT_TRUE(q.try_deq(x));
  EXPECT_EQ(x, 20);
}

// concurrent access test
TEST(BoundedQueue, ConcurrentProducerConsumer) {
  BoundedQueue<int> q(1000);
  constexpr int N = 100'000;

  std::thread producer([&]() {
    for (int i = 0; i < N; i++) {
      while (!q.try_enq(i)) {
      }
    }
  });

  std::thread consumer([&]() {
    for (int i = 0; i < N; i++) {
      int x;
      while (!q.try_deq(x)) {
      }
    }
  });

  producer.join();
  consumer.join();

  int x;
  EXPECT_FALSE(q.try_deq(x));
}

// no lost elements
TEST(BoundedQueue, NoLostElements) {
  const int N = 100'000;
  BoundedQueue<int> q(N);

  std::vector<bool> seen(N + 1, false);

  std::thread producer([&]() {
    for (int i = 1; i <= N; i++) {
      while (!q.try_enq(i)) {
      }
    }
  });

  std::thread consumer([&]() {
    for (size_t i = 0; i < N; i++) {
      int x;
      while (!q.try_deq(x)) {
      }

      ASSERT_TRUE(x >= 1 && x <= N);
      seen[x] = true;
    }
  });

  producer.join();
  consumer.join();

  for (size_t i = 1; i <= N; i++) {
    EXPECT_TRUE(seen[i]);
  }
}

// //no duplicates
// TEST(BoundedQueue, NoDuplicates) {
//     const int N = 5000;
//     BoundedQueue<int> q(N);

//     std::set<int> result;
//     std::mutex m;

//     std::thread producer([&]() {
//         for (int i = 1; i <= N; i++) {
//             while (!q.try_enq(i)) {}
//         }
//     });

//     std::thread consumer([&]() {
//         for (int i = 0; i < N; i++) {
//             std::optional<int> x;
//             while (!(x = q.try_deq())) {}

//             std::lock_guard<std::mutex> lock(m);
//             result.insert(*x);
//         }
//     });

//     producer.join();
//     consumer.join();

//     EXPECT_EQ(result.size(), N);
// }

// //multiple producers and consumers stress test
// TEST(BoundedQueue, MultiThreadedStress) {
//     const int N = 5000;
//     BoundedQueue<int> q(N);

//     std::atomic<int> produced{0};
//     std::atomic<int> consumed{0};

//     auto producer = [&]() {
//         for (;;) {
//             int val = produced.fetch_add(1);
//             if (val >= N) break;

//             while (!q.try_enq(val)) {}
//         }
//     };

//     auto consumer = [&]() {
//         for (;;) {
//             if (consumed.load() >= N) break;

//             std::optional<int> x = q.try_deq();
//             if (x) {
//                 consumed++;
//             }
//         }
//     };

//     std::thread p1(producer), p2(producer);
//     std::thread c1(consumer), c2(consumer);

//     p1.join(); p2.join();
//     c1.join(); c2.join();

//     EXPECT_EQ(consumed.load(), N);
// }

// //FIFO ordering test
// TEST(BoundedQueue, FIFOOrder) {
//     BoundedQueue<int> q(100);

//     for (int i = 0; i < 100; i++) {
//         EXPECT_TRUE(q.try_enq(i));
//     }

//     for (int i = 0; i < 100; i++) {
//         auto x = q.try_deq();
//         ASSERT_TRUE(x.has_value());
//         EXPECT_EQ(*x, i);
//     }
// }