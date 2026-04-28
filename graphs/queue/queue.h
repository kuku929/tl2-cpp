#pragma once
#include "tl2/tl2.h"
#include <vector>
#include <optional>
#include <algorithm>

using namespace tl2;

template<typename T>
class STMQueue {
public:
    STMQueue() : front(std::vector<T>{}),
                 middle(std::vector<T>{}),
                 back(std::vector<T>{}) {}

    // enqueue
    void enqueue(const T& x) {
        atomically([&]() {
            auto b = (std::vector<T>)back;
            b.push_back(x);
            back = b;
        });
    }

    // dequeue
    std::optional<T> dequeue() {
        std::optional<T> result = std::nullopt;

        atomically([&]() {
            auto f = (std::vector<T>)front;

            if (!f.empty()) {
                result = f.front();
                f.erase(f.begin());
                front = f;
                return;
            }

            // move middle → front
            auto m = (std::vector<T>)middle;
            if (!m.empty()) {
                std::reverse(m.begin(), m.end());
                result = m.back();
                m.pop_back();
                front = m;
                middle = std::vector<T>{};
                return;
            }

            // move back → front
            auto b = (std::vector<T>)back;
            if (!b.empty()) {
                std::reverse(b.begin(), b.end());
                result = b.back();
                b.pop_back();
                front = b;
                back = std::vector<T>{};
                return;
            }

            result = std::nullopt;
        });

        return result;
    }

    // size
    size_t size() {
        size_t s = 0;
        atomically([&]() {
            s = ((std::vector<T>)front).size()
              + ((std::vector<T>)middle).size()
              + ((std::vector<T>)back).size();
        });
        return s;
    }

private:
    TVar<std::vector<T>> front;
    TVar<std::vector<T>> middle;
    TVar<std::vector<T>> back;
};