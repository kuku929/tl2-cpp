#pragma once
#include "tl2/tl2.h"
#include <optional>

using namespace tl2;

template <typename T> class STMQueue {
private:
  struct Node {
    T value;
    TVar<Node *> next;

    Node(const T &v) : value(v), next(nullptr) {}
  };

  TVar<Node *> head;
  TVar<Node *> tail;

public:
  STMQueue() : head(nullptr), tail(nullptr) {
    Node *dummy = new Node(T{});
    atomically([&]() {
      head = dummy;
      tail = dummy;
    });
  }

  // enqueue (safe: allocation outside txn)
  void enqueue(const T &x) {
    Node *node = new Node(x); // ✅ OUTSIDE transaction

    atomically([&]() {
      Node *t = static_cast<Node *>(tail);
      t->next = node;
      tail = node;
    });
  }

  // dequeue
  std::optional<T> dequeue() {
    std::optional<T> result = std::nullopt;

    atomically([&]() {
      Node *h = static_cast<Node *>(head);
      Node *next = static_cast<Node *>(h->next);

      if (next == nullptr) {
        result = std::nullopt;
        return;
      }

      result = next->value;
      head = next;
    });

    return result;
  }
};