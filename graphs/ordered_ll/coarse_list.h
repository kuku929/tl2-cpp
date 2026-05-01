#pragma once
#include <climits>
#include <functional>
#include <mutex>
#include <optional>

namespace graphs::linked_list {

template <typename T> class CoarseList {
  struct Node {
    std::optional<T> item;
    int key;
    Node *next;
    Node(std::optional<T> i, int k, Node *n)
        : item(std::move(i)), key(k), next(n) {}
  };

public:
  CoarseList() {
    tail = new Node(std::nullopt, INT_MAX, nullptr);
    tail->next = tail;
    head = new Node(std::nullopt, INT_MIN, tail);
  }

  bool add(const T &item) {
    const int key = std::hash<T>{}(item);
    std::lock_guard<std::mutex> g(lock);
    auto [pred, curr] = locate(key, head);
    if (curr->key == key) {
      return false;
    }
    Node *node = new Node(item, key, curr);
    pred->next = node;
    return true;
  }

  bool remove(const T &item) {
    const int key = std::hash<T>{}(item);
    std::lock_guard<std::mutex> g(lock);
    auto [pred, curr] = locate(key, head);
    if (curr->key == key) {
      pred->next = curr->next;
      return true;
    }
    return false;
  }

  bool contains(const T &item) {
    const int key = std::hash<T>{}(item);
    std::lock_guard<std::mutex> g(lock);
    Node *curr = head->next;
    while (curr->key < key) {
      curr = curr->next;
    }
    return curr->key == key;
  }

private:
  Node *head{};
  Node *tail{};
  std::mutex lock;

  // Returns (pred, curr) where curr->key >= key
  static std::pair<Node *, Node *> locate(int key, Node *start) {
    Node *pred = start;
    Node *curr = pred->next;
    while (curr->key < key) {
      pred = curr;
      curr = curr->next;
    }
    return {pred, curr};
  }
};

} // namespace graphs::linked_list