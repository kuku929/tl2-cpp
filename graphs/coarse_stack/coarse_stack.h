#pragma once
#include "stm.h"
#include "tl2/tl2.h"
#include <optional>

/*
NOTE: We are not freeing the nodes which are popped. It can lead to memory leak!
It's acceptable since this is just for testing STM
*/

template <typename T> class StackSTM {
private:
  struct Node {
    T value;
    Node *next;

    Node(T v, Node *n) : value(v), next(n) {}
  };

  tl2::TVar<Node *> top;

public:
  StackSTM() : top(nullptr) {}

  void push(const T &x) {
    tl2::atomically([&]() {
      Node *old_top = static_cast<Node *>(top);
      Node *node = new Node(x, old_top);
      top = node;
    });
  }

  std::optional<T> try_pop() {
    std::optional<T> result = std::nullopt;

    tl2::atomically([&]() {
      Node *old_top = static_cast<Node *>(top);

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

    tl2::atomically(
        [&]() { is_empty = (static_cast<Node *>(top) == nullptr); });

    return is_empty;
  }
};