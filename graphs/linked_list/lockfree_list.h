#pragma once
#include <atomic>
#include <climits>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>

namespace graphs::linked_list {

template <typename T> class LockFreeList {
  struct Node;

  // AtomicMarkableReference-style pointer: LSB is mark bit
  struct AMR {
    std::atomic<std::uintptr_t> raw;

    explicit AMR(Node* ptr, bool mark = false) : raw(pack(ptr, mark)) {}

    void set(Node* ptr, bool mark) {
      raw.store(pack(ptr, mark), std::memory_order_release);
    }

    static std::uintptr_t pack(Node* ptr, bool mark) {
      auto p = reinterpret_cast<std::uintptr_t>(ptr);
      return (p & ~std::uintptr_t{1}) | (mark ? 1 : 0);
    }

    static Node* get_ptr(std::uintptr_t v) {
      return reinterpret_cast<Node*>(v & ~std::uintptr_t{1});
    }

    static bool get_mark(std::uintptr_t v) { return (v & 1) != 0; }

    Node* get_reference() const {
      return get_ptr(raw.load(std::memory_order_acquire));
    }

    Node* get(bool& marked) const {
      auto v = raw.load(std::memory_order_acquire);
      marked = get_mark(v);
      return get_ptr(v);
    }

    bool compare_and_set(Node* expected_ref, Node* new_ref, bool expected_mark, bool new_mark) {
      auto expected = pack(expected_ref, expected_mark);
      return raw.compare_exchange_strong(
          expected, pack(new_ref, new_mark), std::memory_order_acq_rel, std::memory_order_acquire);
    }
  };

  struct Node {
    std::optional<T> item;
    int key;
    AMR next;
    Node(std::optional<T> i, int k, Node* n) : item(std::move(i)), key(k), next(n, false) {}
  };

public:
  LockFreeList() {
    tail = new Node(std::nullopt, INT_MAX, nullptr);
    tail->next.set(tail, false);
    head = new Node(std::nullopt, INT_MIN, tail);
  }

  bool add(const T& item) {
    const int key = std::hash<T>{}(item);
    for (;;) {
      auto [pred, curr] = locate(head, key);
      if (curr->key == key) {
        return false;
      }
      Node* node = new Node(item, key, curr);
      if (pred->next.compare_and_set(curr, node, false, false)) {
        return true;
      }
    }
  }

  bool remove(const T& item) {
    const int key = std::hash<T>{}(item);
    for (;;) {
      auto [pred, curr] = locate(head, key);
      if (curr->key != key) {
        return false;
      }
      bool marked = false;
      Node* succ = curr->next.get(marked);
      if (!marked && curr->next.compare_and_set(succ, succ, false, true)) {
        pred->next.compare_and_set(curr, succ, false, false);
        return true;
      }
    }
  }

  bool contains(const T& item) {
    const int key = std::hash<T>{}(item);
    bool marked = false;
    Node* curr = head;
    for (;;) {
      Node* next = curr->next.get(marked);
      if (!marked) {
        curr = next;
        if (curr->key >= key) {
          return curr->key == key;
        }
      } else {
        curr = next;
      }
    }
  }

private:
  Node* head{};
  Node* tail{};

  // Returns (pred, curr) where pred and curr are adjacent and unmarked.
  std::pair<Node*, Node*> locate(Node* start, int key) {
    for (;;) {
      Node* pred = start;
      bool marked = false;
      Node* curr = pred->next.get(marked);

      for (;;) {
        Node* succ = curr->next.get(marked);
        if (marked) {
          if (!pred->next.compare_and_set(curr, succ, false, false)) {
            break; // retry
          }
          curr = succ;
        } else if (curr->key >= key) {
          return {pred, curr};
        } else {
          pred = curr;
          curr = succ;
        }
      }
    }
  }
};

} // namespace graphs::linked_list