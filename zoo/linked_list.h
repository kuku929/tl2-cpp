#pragma once
#include "stm.h"
#include "tl2/tl2.h"
#include <climits>
#include <functional>
#include <memory>
#include <utility>

namespace zoo {

template <typename T> class LinkedList {
  struct Node {
    tl2::TVar<T> item;
    tl2::TVar<Node *> next;
    Node(const T &v, Node *n) : item(v), next(n) {}
    Node(const T &v) : item(v), next(nullptr) {}
  };

public:
  LinkedList() : m_head(nullptr), m_sz(0) { ; }

  ~LinkedList() {
    Node *curr = m_head.unsafe_get();
    while (curr != nullptr) {
      Node *next = curr->next.unsafe_get();
      delete curr;
      curr = next;
    }
  }

  void add(const T &item) {
    // this is a bug. If add() is called 
    // inside a transaction then this allocation
    // will never free. This is a bug and will
    // cause an error with Asan.
    Node * node = new Node(item);
    tl2::atomically([&]() {
      Node *head_val = static_cast<Node *>(m_head);
      node->next = head_val;
      m_head = node.get();
      m_sz = static_cast<std::size_t>(m_sz) + 1;
    });
  }

  bool remove(const T &item) {
    bool result = false;
    Node *node = tl2::atomically(
        [&]() {
          Node *head = static_cast<Node *>(m_head);
          auto [pred, curr] = _locate(head, item);
          if (curr != nullptr) {
            m_sz = static_cast<std::size_t>(m_sz) - 1;
            if (curr != head) {
              pred->next = static_cast<Node *>(curr->next);
              result = true;
              return curr;
            } else {
              Node *old_head = static_cast<Node *>(m_head);
              Node *next_head = static_cast<Node *>(old_head->next);
              m_head = next_head;
              result = true;
              return old_head;
            }
          }
          return static_cast<Node *>(nullptr);
        },
        result);
    delete node;
    return result;
  }

  bool contains(const T &item) {
    bool result = false;
    tl2::atomically(
        [&]() {
          Node *curr = static_cast<Node *>(m_head);
          while (curr != nullptr && static_cast<T>(curr->item) != item) {
            curr = static_cast<Node *>(curr->next);
          }
          result = (curr != nullptr);
        },
        result);
    return result;
  }

  std::optional<T> get(const T &item) {
    return tl2::atomically([&]() {
      Node *curr = static_cast<Node *>(m_head);
      while (curr != nullptr && static_cast<T>(curr->item) != item) {
        curr = static_cast<Node *>(curr->next);
      }
      return (curr != nullptr) ? std::optional<T>(static_cast<T>(curr->item))
                               : std::nullopt;
    });
  }

  void update(const T &curr_item, const T &next_item) {
    tl2::atomically([&]() {
      Node *curr = static_cast<Node *>(m_head);
      while (curr != nullptr && static_cast<T>(curr->item) != curr_item) {
        curr = static_cast<Node *>(curr->next);
      }
      if (curr != nullptr) {
        curr->item = next_item;
      } else {
        add(next_item);
      }
    });
  }

  std::size_t size() {
    return tl2::atomically(
        [this]() { return static_cast<std::size_t>(this->m_sz); });
  }

private:
  tl2::TVar<Node *> m_head;
  tl2::TVar<std::size_t> m_sz;

  // Must be called only inside a transaction
  inline std::pair<Node *, Node *> _locate(Node *start, const T &item) {
    if (start == nullptr) {
      return {nullptr, nullptr};
    }
    Node *pred = nullptr;
    Node *curr = start;
    while (curr != nullptr && static_cast<T>(curr->item) != item) {
      pred = curr;
      curr = static_cast<Node *>(curr->next);
    }
    return {pred, curr};
  }
};

} // namespace zoo