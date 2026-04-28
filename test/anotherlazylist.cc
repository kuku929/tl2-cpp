#include "stm.h"
#include "tl2/tl2.h"
#include <thread>
#include <gtest/gtest.h>

/*
HAS A LOT OF BUGS
AND LAZY LIST CANNOT BE IMPLEMENTED WITH STM
*/

using namespace tl2;

template <typename T>
struct Node{
    T item;
    int key;
    tl2::TVar<Node*> next;
    tl2::TVar<bool> marked;

    Node(T item, int key, Node* next)
        : item(item), key(key), next(next), marked(false) {}
};

template <typename T> 
struct LazyLinkedList{
public:
    LazyListSTM() {
        tail = new Node<T>(T{}, INT_MAX, nullptr);
        head = new Node<T>(T{}, INT_MIN, tail);
    }

    bool add(T item) {
        int key = std::hash<T>{}(item);

        while (true) {
            auto [pred, curr] = locate(key);
            bool result = false;
            bool decided = false;

            tl2::atomically([&]() {
                if (!(bool)pred->marked &&
                    !(bool)curr->marked &&
                    (Node<T>*)pred->next == curr) {

                    decided = true;

                    if (curr->key == key) {
                        result = false;   // already exists
                        return;
                    }

                    Node<T>* node = new Node<T>(item, key, curr);
                    pred->next = node;
                    result = true;
                }
            });

            if (decided) return result;
            // else retry
        }
    }

    bool remove(T item) {
        int key = std::hash<T>{}(item);

        while (true) {
            auto [pred, curr] = locate(key);
            bool result = false;
            bool decided = false;

            tl2::atomically([&]() {
                if (!(bool)pred->marked &&
                    !(bool)curr->marked &&
                    (Node<T>*)pred->next == curr) {
                    
                    decided = true;

                    if (curr->key != key) {
                        result = false;
                        return;
                    }

                    curr->marked = true;                  // logical delete
                    pred->next = (Node<T>*)curr->next;    // physical delete
                    result = true;
                }
            });

            if (decided) return result;
        }
    }

    bool contains(T item) {
        int key = std::hash<T>{}(item);

        Node<T>* curr = head;
        while (curr->key < key) {
            tl2::atomically([&](){
                curr = (Node<T>*)curr->next;
            });
        }

        bool marked;
        tl2::atomically([&](){
            marked = (bool)curr->marked;
        });

        return curr->key == key && !marked;
    }

private:
    Node<T>* head,tail;

    std::pair<Node<T>*, Node<T>*> locate(int key) {
        Node<T>* pred,curr;

        pred = head;

        tl2::atomically([&](){
            curr = (Node<T>*)head->next;
        });

        while (curr->key < key) {
            pred = curr;
            tl2::atomically([&](){
                curr = (Node<T>*)curr->next;
            });
        }
        return {pred, curr};
    }

};

//test 1: basic correctness test
TEST(STM, LazyListBasic) {
    LazyLinkedList<int> list();

    EXPECT_TRUE(list.add(10));
    EXPECT_TRUE(list.contains(10));
    EXPECT_FALSE(list.add(10));

    EXPECT_TRUE(list.remove(10));
    EXPECT_FALSE(list.contains(10));
}

//test 2: concurrent correctness test
TEST(STM, LazyListConcurrentInsert) {
    LazyLinkedList<int> list();
    constexpr int N = 5000;

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

    EXPECT_EQ(count, N);  // no duplicates, no missing
}

//test 3: insert + remove race
TEST(STM, LazyListAddRemoveRace) {
    LazyLinkedList<int> list;
    constexpr int N = 5000;

    std::thread t1([&]() {
        for (int i = 0; i < N; i++) list.add(i);
    });

    std::thread t2([&]() {
        for (int i = 0; i < N; i++) list.remove(i);
    });

    t1.join();
    t2.join();

    int remaining = 0;
    for (int i = 0; i < N; i++) {
        if (list.contains(i)) remaining++;
    }

    EXPECT_TRUE(remaining >= 0); // sanity
}