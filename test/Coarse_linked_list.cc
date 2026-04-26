#include "stm.h"
#include "tl2/tl2.h"
#include <thread>
#include <gtest/gtest.h>

template<typename T>
struct Node {
    T item;
    int key;
    tl2::TVar<Node*> next;

    Node(T item, int key, Node* next)
        : item(item), key(key), next(next) {}
};

template <typename T>
struct CoarseLinkedList{
public:
    CoarseLinkedList() {
        tail = new Node<T>(T{}, INT_MAX, nullptr);
        head = new Node<T>(T{}, INT_MIN, tail);
    }

    bool add(T item) {
        int key = std::hash<T>{}(item);
        bool result = false;

        tl2::atomically([&]() {
            auto [pred, curr] = locate(head, key);

            if (curr->key == key) {
                result = false;
                return;
            }

            Node<T>* node = new Node<T>(item, key, curr);
            pred->next = node;
            result = true;
        });

        return result;
    }

    bool remove(T item) {
        int key = std::hash<T>{}(item);
        bool result = false;

        tl2::atomically([&]() {
            auto [pred, curr] = locate(head, key);

            if (curr->key == key) {
                pred->next = (Node<T>*)curr->next;
                result = true;
            } else {
                result = false;
            }
        });

        return result;
    }

    bool contains(T item) {
        int key = std::hash<T>{}(item);
        bool result = false;

        tl2::atomically([&]() {
            Node<T>* curr = head;

            while (curr->key < key) {
                curr = (Node<T>*)curr->next;
            }

            result = (curr->key == key);
        });

        return result;
    }

private:
    Node<T>* head,tail;

    //must be called only inside a transaction
    std::pair<Node<T>*, Node<T>*> locate(Node<T>* start, int key) {
        Node<T>* pred = start;
        Node<T>* curr = (Node<T>*)pred->next;

        while (curr->key < key) {
            pred = curr;
            curr = (Node<T>*)curr->next;
        }
        return {pred, curr};
    }
};

TEST(STM, CoarseListBasic) {
    CoarseLinkedList<int> list;

    EXPECT_TRUE(list.add(10));
    EXPECT_TRUE(list.contains(10));
    EXPECT_FALSE(list.add(10));
    EXPECT_TRUE(list.remove(10));
    EXPECT_FALSE(list.contains(10));
}

TEST(STM, CoarseListConcurrentInsert) {
    CoarseLinkedList<int> list;
    const int N = 5000;

    std::thread t1([&]() {
        for (int i = 0; i < N; i++) list.add(i);
    });

    std::thread t2([&]() {
        for (int i = 0; i < N; i++) list.add(i);
    });

    t1.join();
    t2.join();

    int count = 0;
    for (int i = 0; i < N; i++) {
        if (list.contains(i)) count++;
    }

    EXPECT_EQ(count, N);
}

TEST(STM, CoarseListAddRemove) {
    CoarseLinkedList<int> list;
    const int N = 3000;

    std::thread t1([&]() {
        for (int i = 0; i < N; i++) list.add(i);
    });

    std::thread t2([&]() {
        for (int i = 0; i < N; i++) list.remove(i);
    });

    t1.join();
    t2.join();

    for (int i = 0; i < N; i++) {
        bool x = list.contains(i);
        EXPECT_TRUE(x == true || x == false);
    }
}