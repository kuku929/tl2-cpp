#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "coarse_linked_list.h"

using namespace std;

void benchmark_batch(int threads, int ops) {
    CoarseLinkedList<int> list;

    auto start = chrono::high_resolution_clock::now();

    vector<thread> ts;

    for (int t = 0; t < threads; t++) {
        ts.emplace_back([&]() {
            for (int i = 0; i < ops; i++) {
                list.add(i);
                list.remove(i);
            }
        });
    }

    for (auto &th : ts) th.join();

    auto end = chrono::high_resolution_clock::now();

    double sec = chrono::duration<double>(end - start).count();
    double throughput = (static_cast<double>(threads * ops)) / sec;

    cout << threads << "," << throughput << endl;
}

int main() {
    int ops = 100000;
    vector<int> thread_counts = {1, 2, 4, 8};

    cout << "threads,throughput" << endl;

    for (int t : thread_counts) {
        benchmark_batch(t, ops);
    }

    return 0;
}