#pragma once
#include <thread>
#include <vector>
#include <optional>
#include <set>
#include <mutex> //used for tests
#include <gtest/gtest.h>
#include "tl2/tl2.h"

using namespace tl2;

template <typename T>
struct BoundedQueue{
public:
    BoundedQueue(size_t cap) 
      : _capacity(cap),
        items(cap,TVar<T>(T())),
        head(0),
        tail(0) {}

    bool try_enq(const T& x){
        bool success=false;

        tl2::atomically([&]() {
            size_t h = static_cast<size_t>(head);
            size_t t = static_cast<size_t>(tail);

            if (t - h == _capacity) {
                success = false;
                return;
            }

            items[t % _capacity] = x;
            tail = t + 1;
            success = true;
        });

        return success;
    }

    //returns std::nullopt if the queue is empty
    std::optional<T> try_deq() {
        std::optional<T> result = std::nullopt;

        tl2::atomically([&]() {
            size_t h = static_cast<size_t>(head);
            size_t t = static_cast<size_t>(tail);


            if (t == h) {
                result = std::nullopt;
                return;
            }

            result = static_cast<T>(items[h % _capacity]);
            head = h + 1;
        });

        return result;
    }

    size_t capacity() const{
        return _capacity;
    }

    size_t size() const{
        size_t size;
        tl2::atomically([&](){
            size = static_cast<size_t>(tail) - static_cast<size_t>(head);
        });
        return size;
    }

private:
    const size_t _capacity;
    std::vector<TVar<T>> items;
    TVar<size_t> head,tail;
};