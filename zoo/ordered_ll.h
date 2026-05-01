#pragma once
#include "tl2/tl2.h"
#include <climits>
#include <functional>
#include <utility>

namespace zoo {

template <typename T, typename HashFunc> class OrderedLinkedList {
  struct Node {
    T item;
    int key;
    tl2::TVar<Node *> next;
    Node(const T &v, int k, Node *n) : item(v), key(k), next(n) {}
  };

public:
  OrderedLinkedList() : hash(std::hash<T>()) {
    m_tail = new Node(T{}, INT_MAX, nullptr);
    m_head = new Node(T{}, INT_MIN, m_tail);
  }

  OrderedLinkedList(HashFunc f) : hash(f) {
    m_tail = new Node(T{}, INT_MAX, nullptr);
    m_head = new Node(T{}, INT_MIN, m_tail);
  }

  ~OrderedLinkedList() {
    delete m_head; m_head = nullptr;
    delete m_tail; m_tail = nullptr;
  }

  bool add(const T &item) {
    bool result = false;
    int key = hash(item);
    tl2::atomically([&]() {
      auto [pred, curr] = locate(m_head, key);
      if (curr->key == key) {
        result = false;
        return;
      }
      auto *node = new Node(item, key, curr);
      pred->next = node;
      result = true;
    });
    return result;
  }

  bool remove(const T &item) {
    bool result = false;
    int key = hash(item);
    tl2::atomically([&]() {
      auto [pred, curr] = locate(m_head, key);
      if (curr->key == key) {
        pred->next = static_cast<Node *>(curr->next);
        result = true;
      }
    }, result);
    return result;
  }

  bool contains(const T &item) {
    bool result = false;
    int key = hash(item);
    tl2::atomically([&]() {
      Node *curr = m_head;
      while (curr->key < key) {
        curr = static_cast<Node *>(curr->next);
      }
      result = (curr->key == key);
    }, result);
    return result;
  }

private:
  Node *m_head{};
  Node *m_tail{};
  HashFunc hash;

  // Must be called only inside a transaction
  inline std::pair<Node *, Node *> locate(Node *start, int key) {
    Node *pred = start;
    Node *curr = static_cast<Node *>(pred->next);
    while (curr->key < key) {
      pred = curr;
      curr = static_cast<Node *>(curr->next);
    }
    return {pred, curr};
  }
};

} // namespace zoo