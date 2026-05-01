#pragma once
#include "tl2/tl2.h"
#include <optional>
#include <set>
#include <thread>
#include <vector>

using namespace tl2;

template <typename T> struct BoundedQueue {
public:
  BoundedQueue(size_t cap)
      : _capacity(cap), items(cap, TVar<T>(T())), head(0), tail(0) {}

  bool try_enq(const T &x) {
    bool success = false;

    tl2::atomically([&]() { success = enq_tx(x); });

    return success;
  }

  // returns false if the queue is empty, otherwise returns true and sets out
  bool try_deq(T &out) {
    bool success = false;

    tl2::atomically([&]() {
      auto res = deq_tx();
      if (res.first) {
        out = res.second;
        success = true;
      }
    });

    return success;
  }

  size_t capacity() const { return _capacity; }

  size_t size() const {
    size_t size;
    tl2::atomically([&]() {
      size = static_cast<size_t>(tail) - static_cast<size_t>(head);
    });
    return size;
  }

  // Batch enqueue/dequeue in a single transaction for better performance
  // Usage: atomically([&]() { q.enq_tx(x); q.deq_tx(); });
  bool enq_tx(const T &x) {
    size_t h = static_cast<size_t>(head);
    size_t t = static_cast<size_t>(tail);

    if (t - h == _capacity) {
      return false;
    }

    items[t % _capacity] = x;
    tail = t + 1;
    return true;
  }

  std::pair<bool, T> deq_tx() {
    size_t h = static_cast<size_t>(head);
    size_t t = static_cast<size_t>(tail);

    if (t == h) {
      return {false, T()};
    }

    auto result = static_cast<T>(items[h % _capacity]);
    head = h + 1;
    return {true, result};
  }

private:
  const size_t _capacity;
  std::vector<TVar<T>> items;
  TVar<size_t> head, tail;
};