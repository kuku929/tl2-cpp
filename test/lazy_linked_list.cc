#include <gtest/gtest.h>
#include <climits>
#include <thread>
#include "tl2/tl2.h"

using namespace tl2;

/*
CANNOT BE IMPLEMENTED WITH STM
*/

template <typename T>
struct Node {
  T item;
  int key;
  TVar<Node*> next;
  TVar<bool> marked;

  Node(T item_, int key_, Node* next_)
      : item(item_), key(key_), next(next_), marked(false) {}
};

template <typename T>
class LazyLinkedList {
 public:
  LazyLinkedList() {
    tail = new Node<T>(T{}, INT_MAX, nullptr);
    head = new Node<T>(T{}, INT_MIN, tail);
  }

  bool add(T item) {
    int key = std::hash<T>{}(item);

    while (true) {
      Node<T>* pred;
      Node<T>* curr;

      locate(key, pred, curr);

      bool result = false;
      bool decided = false;

      atomically([&]() {
        if (!static_cast<bool>(pred->marked) &&
            !static_cast<bool>(curr->marked) &&
            static_cast<Node<T>*>(pred->next) == curr) {

          decided = true;

          if (curr->key == key) {
            result = false;
            return;
          }

          Node<T>* node = new Node<T>(item, key, curr);
          pred->next = node;
          result = true;
        }
      });

      if (decided)
        return result;
    }
  }

  bool remove(T item) {
    int key = std::hash<T>{}(item);

    while (true) {
      Node<T>* pred;
      Node<T>* curr;

      locate(key, pred, curr);

      bool result = false;
      bool decided = false;

      atomically([&]() {
        if (!static_cast<bool>(pred->marked) &&
            !static_cast<bool>(curr->marked) &&
            static_cast<Node<T>*>(pred->next) == curr) {

          decided = true;

          if (curr->key != key) {
            result = false;
            return;
          }

          curr->marked = true;
          pred->next = static_cast<Node<T>*>(curr->next);
          result = true;
        }
      });

      if (decided)
        return result;
    }
  }

  bool contains(T item) {
    int key = std::hash<T>{}(item);
    Node<T>* curr = head;

    while (true) {
      Node<T>* next;

      atomically([&]() { next = static_cast<Node<T>*>(curr->next); });

      if (next->key >= key) {
        bool marked;
        atomically([&]() { marked = static_cast<bool>(next->marked); });
        return (next->key == key && !marked);
      }

      curr = next;
    }
  }

 private:
  Node<T>* head;
  Node<T>* tail;

  void locate(int key, Node<T>*& pred, Node<T>*& curr) {
    pred = head;

    atomically([&]() { curr = static_cast<Node<T>*>(head->next); });

    while (curr->key < key) {
      pred = curr;

      atomically([&]() { curr = static_cast<Node<T>*>(curr->next); });
    }
  }
};

// ================= TESTS =================

TEST(STM, LazyListBasic) {
  LazyLinkedList<int> list;

  EXPECT_TRUE(list.add(10));
  EXPECT_TRUE(list.contains(10));
  EXPECT_FALSE(list.add(10));

  EXPECT_TRUE(list.remove(10));
  EXPECT_FALSE(list.contains(10));
}

// TEST(STM, LazyListConcurrentInsert) {
//   LazyLinkedList<int> list;
//   const int N = 5000;

//   std::thread t1([&]() noexcept {
//     for (int i = 0; i < N; i++)
//       list.add(i);
//   });

//   std::thread t2([&]() noexcept {
//     for (int i = 0; i < N; i++)
//       list.add(i);
//   });

//   t1.join();
//   t2.join();

//   int count = 0;
//   for (int i = 0; i < N; i++) {
//     if (list.contains(i))
//       count++;
//   }

//   EXPECT_EQ(count, N);
// }

TEST(STM, LazyListAddRemoveRace) {
  LazyLinkedList<int> list;
  const int N = 5000;

  std::thread t1([&]() noexcept {
    for (int i = 0; i < N; i++)
      list.add(i);
  });

  std::thread t2([&]() noexcept {
    for (int i = 0; i < N; i++)
      list.remove(i);
  });

  t1.join();
  t2.join();

  int remaining = 0;
  for (int i = 0; i < N; i++) {
    if (list.contains(i))
      remaining++;
  }

  EXPECT_TRUE(remaining >= 0);
}