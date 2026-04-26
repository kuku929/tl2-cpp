#include "stm.h"
#include "tl2/tl2.h"
#include <optional>
#include <gtest/gtest.h>

/*
NOTE: We are not freeing the nodes which are popped. It can lead to memory leak!
It's acceptable since this is just for testing STM
*/

template <typename T>
class StackSTM {
private:
    struct Node {
        T value;
        Node* next;

        Node(T v, Node* n) : value(v), next(n) {}
    };

    tl2::TVar<Node*> top;

public:
    StackSTM() : top(nullptr) {}

    void push(const T& x) {
        tl2::atomically([&]() {
            Node* old_top = (Node*)top;
            Node* node = new Node(x, old_top);
            top = node;
        });
    }

    std::optional<T> try_pop() {
        std::optional<T> result = std::nullopt;

        tl2::atomically([&]() {
            Node* old_top = (Node*)top;

            if (old_top == nullptr) {
                result = std::nullopt;
                return;
            }

            result = old_top->value;
            top = old_top->next;
        });

        return result;
    }

    bool empty() {
        bool is_empty = false;

        tl2::atomically([&]() {
            is_empty = ((Node*)top == nullptr);
        });

        return is_empty;
    }
};

//test 1: testing basic functionality
TEST(StackSTM, BasicPushPop) {
    StackSTM<int> s;

    s.push(10);
    s.push(20);

    auto x = s.try_pop();
    ASSERT_TRUE(x.has_value());
    EXPECT_EQ(*x, 20);

    x = s.try_pop();
    ASSERT_TRUE(x.has_value());
    EXPECT_EQ(*x, 10);
}

//test 2: empty behaviour
TEST(StackSTM, EmptyPop) {
    StackSTM<int> s;

    auto x = s.try_pop();
    EXPECT_FALSE(x.has_value());
}

//test 3: concurrent push
TEST(StackSTM, ConcurrentPush) {
    StackSTM<int> s;
    const int N = 5000;

    std::thread t1([&]() {
        for (int i = 0; i < N; i++) s.push(i);
    });

    std::thread t2([&]() {
        for (int i = N; i < 2*N; i++) s.push(i);
    });

    t1.join();
    t2.join();

    int count = 0;
    while (s.try_pop()) count++;

    EXPECT_EQ(count, 2 * N);
}

//test 4: push/pop race
TEST(StackSTM, PushPopRace) {
    StackSTM<int> s;
    const int N = 5000;

    std::atomic<int> pushed{0};
    std::atomic<int> popped{0};

    std::thread t1([&]() {
        for (int i = 0; i < N; i++) {
            s.push(i);
            pushed++;
        }
    });

    std::thread t2([&]() {
        for (int i = 0; i < N; i++) {
            auto x = s.try_pop();
            if (x) popped++;
        }
    });

    t1.join();
    t2.join();

    EXPECT_LE(popped.load(), pushed.load());
}

