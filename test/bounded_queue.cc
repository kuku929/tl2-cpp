#include <thread>
#include <vector>
#include <optional>
#include <set>
#include <mutex> //used for tests
#include <gtest/gtest.h>
#include "tl2/tl2.h"

using namespace tl2;

template <typename T>
struct BoundedQueue{
public:
    BoundedQueue(size_t cap) 
      : _capacity(cap),
        items(cap,TVar<T>(T())),
        head(0),
        tail(0) {}

    bool try_enq(const T& x){
        bool success=false;

        tl2::atomically([&]() {
            size_t h = static_cast<size_t>(head);
            size_t t = static_cast<size_t>(tail);

            if (t - h == _capacity) {
                success = false;
                return;
            }

            items[t % _capacity] = x;
            tail = t + 1;
            success = true;
        });

        return success;
    }

    //returns std::nullopt if the queue is empty
    std::optional<T> try_deq() {
        std::optional<T> result = std::nullopt;

        tl2::atomically([&]() {
            size_t h = static_cast<size_t>(head);
            size_t t = static_cast<size_t>(tail);


            if (t == h) {
                result = std::nullopt;
                return;
            }

            result = static_cast<T>(items[h % _capacity]);
            head = h + 1;
        });

        return result;
    }

    size_t capacity() const{
        return _capacity;
    }

    size_t size() const{
        size_t size;
        tl2::atomically([&](){
            size = static_cast<size_t>(tail) - static_cast<size_t>(head);
        });
        return size;
    }

private:
    const size_t _capacity;
    std::vector<TVar<T>> items;
    TVar<size_t> head,tail;
};

//testing the basic functionality of the queue
//enqueue two values and then dequeue them
TEST(BoundedQueue, EnqueueDequeueSingle) {
    BoundedQueue<int> q(5);

    EXPECT_TRUE(q.try_enq(10));
    EXPECT_TRUE(q.try_enq(20));

    auto x = q.try_deq();
    ASSERT_TRUE(x.has_value());
    EXPECT_EQ(*x, 10);

    x = q.try_deq();
    ASSERT_TRUE(x.has_value());
    EXPECT_EQ(*x, 20);
}

//concurrent access test
TEST(BoundedQueue, ConcurrentProducerConsumer) {
    BoundedQueue<int> q(1000);
    constexpr int N = 1000;

    std::thread producer([&]() {
        for (int i = 0; i < N; i++) {
            while (!q.try_enq(i)) {}
        }
    });

    std::thread consumer([&]() {
        for (int i = 0; i < N; i++) {
            std::optional<int> x;
            while (!(x = q.try_deq())) {}
        }
    });

    producer.join();
    consumer.join();

    auto x = q.try_deq();
    EXPECT_FALSE(x.has_value());
}

//no lost elements
TEST(BoundedQueue, NoLostElements) {
    const int N = 10000;
    BoundedQueue<int> q(N);

    std::vector<bool> seen(N + 1, false);

    std::thread producer([&]() {
        for (int i = 1; i <= N; i++) {
            while (!q.try_enq(i)) {}
        }
    });

    std::thread consumer([&]() {
        for (size_t i = 0; i < N; i++) {
            std::optional<size_t> x;
            while (!(x = q.try_deq())) {}

            ASSERT_TRUE(*x >= 1 && *x <= N);
            seen[*x] = true;
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