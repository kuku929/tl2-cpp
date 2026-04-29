#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "tl2/tl2.h"

using namespace std;

// shared counter (transactional)
tl2::TVar<long long> counter(0);

void benchmark(int threads, int ops_per_thread) {
    // reset counter
    tl2::atomically([&]() {
        counter = 0;
    });

    vector<thread> ts;
    
    auto start = chrono::high_resolution_clock::now();

    for (int t = 0; t < threads; t++) {
        ts.emplace_back([&]() {
            for (int i = 0; i < ops_per_thread; i++) {
                tl2::atomically([&]() {
                    counter = static_cast<long long>(counter) + 1;
                });
            }
        });
    }

    for (auto &th : ts) th.join();

    auto end = chrono::high_resolution_clock::now();

    double sec = chrono::duration<double>(end - start).count();

    double throughput = static_cast<double>(threads * ops_per_thread) / sec;

    cout << threads << "," << throughput << endl;
}

int main() {
    int ops = 100000;

    vector<int> thread_counts = {1, 2, 4, 8, 12};

    cout << "threads,throughput" << endl;

    for (int t : thread_counts) {
        benchmark(t, ops);
    }

    return 0;
}