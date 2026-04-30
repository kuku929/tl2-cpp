#pragma once
#include "tl2/tl2.h"
#include <climits>
#include <functional>
#include <utility>

namespace zoo {

template <typename T> class ConcurrentLinkedList {
  struct Node {
    T item;
    int key;
    tl2::TVar<Node*> next;
    Node(const T& v, int k, Node* n) : item(v), key(k), next(n) {}
  };

public:
  ConcurrentLinkedList() {
    m_tail = new Node(T{}, INT_MAX, nullptr);
    m_head = new Node(T{}, INT_MIN, m_tail);
  }

  bool add(const T& item) {
    bool result = false;
    tl2::atomically([&]() {
      int key = std::hash<T>{}(item);
      auto [pred, curr] = locate(m_head, key);
      if (curr->key == key) {
        result = false;
        return;
      }
      auto* node = new Node(item, key, curr);
      pred->next = node;
      result = true;
    });
    return result;
  }

  bool remove(const T& item) {
    bool result = false;
    tl2::atomically([&]() {
      int key = std::hash<T>{}(item);
      auto [pred, curr] = locate(m_head, key);
      if (curr->key == key) {
        pred->next = static_cast<Node*>(curr->next);
        result = true;
      } else {
        result = false;
      }
    });
    return result;
  }

  bool contains(const T& item) {
    bool result = false;
    tl2::atomically([&]() {
      int key = std::hash<T>{}(item);
      Node* curr = m_head;
      while (curr->key < key) {
        curr = static_cast<Node*>(curr->next);
      }
      result = (curr->key == key);
    });
    return result;
  }

private:
  Node* m_head{};
  Node* m_tail{};

  // Must be called only inside a transaction
  std::pair<Node*, Node*> locate(Node* start, int key) {
    Node* pred = start;
    Node* curr = static_cast<Node*>(pred->next);
    while (curr->key < key) {
      pred = curr;
      curr = static_cast<Node*>(curr->next);
    }
    return {pred, curr};
  }
};

} // namespace zoo