#pragma once
#include <climits>
#include <functional>
#include <mutex>
#include <optional>

namespace graphs::linked_list {

template <typename T> class FineList {
  struct Node {
    std::optional<T> item;
    int key;
    Node* next;
    std::mutex lock;
    Node(std::optional<T> i, int k, Node* n) : item(std::move(i)), key(k), next(n) {}
  };

public:
  FineList() {
    tail = new Node(std::nullopt, INT_MAX, nullptr);
    tail->next = tail;
    head = new Node(std::nullopt, INT_MIN, tail);
  }

  bool add(const T& item) {
    const int key = std::hash<T>{}(item);

    Node* pred = head;
    pred->lock.lock();
    Node* curr = pred->next;
    curr->lock.lock();

    while (curr->key < key) {
      pred->lock.unlock();
      pred = curr;
      curr = curr->next;
      curr->lock.lock();
    }

    bool result = false;
    if (curr->key == key) {
      result = false;
    } else {
      Node* node = new Node(item, key, curr);
      pred->next = node;
      result = true;
    }

    curr->lock.unlock();
    pred->lock.unlock();
    return result;
  }

  bool remove(const T& item) {
    const int key = std::hash<T>{}(item);

    Node* pred = head;
    pred->lock.lock();
    Node* curr = pred->next;
    curr->lock.lock();

    while (curr->key < key) {
      pred->lock.unlock();
      pred = curr;
      curr = curr->next;
      curr->lock.lock();
    }

    bool result = false;
    if (curr->key == key) {
      pred->next = curr->next;
      result = true;
    } else {
      result = false;
    }

    curr->lock.unlock();
    pred->lock.unlock();
    return result;
  }

  bool contains(const T& item) {
    const int key = std::hash<T>{}(item);

    Node* pred = head;
    pred->lock.lock();
    Node* curr = pred->next;
    curr->lock.lock();

    while (curr->key < key) {
      pred->lock.unlock();
      pred = curr;
      curr = curr->next;
      curr->lock.lock();
    }

    bool result = (curr->key == key);

    curr->lock.unlock();
    pred->lock.unlock();
    return result;
  }

private:
  Node* head{};
  Node* tail{};
};

} // namespace graphs::linked_list