#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <optional>
#include "queue.h"

using namespace std;
using namespace tl2;


void benchmark(int threads, int ops_per_thread) {
    STMQueue<int> q;

    auto start = chrono::high_resolution_clock::now();

    vector<thread> ts;

    for (int t = 0; t < threads; t++) {
        ts.emplace_back([&]() {
            for (int i = 0; i < ops_per_thread; i++) {
                if (i % 2 == 0)
                    q.enqueue(i);
                else
                    q.dequeue();
            }
        });
    }

    for (auto &t : ts) t.join();

    auto end = chrono::high_resolution_clock::now();

    double sec = chrono::duration<double>(end - start).count();
    double total_ops = (double)threads * ops_per_thread;

    cout << threads << "," << (total_ops / sec) << endl;
}

int main() {
    cout << "threads,throughput\n";

    for (int t = 1; t <= 8; t *= 2) {
        benchmark(t, 100000);
    }
}