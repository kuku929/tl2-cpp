#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "bounded_queue.h"  // your TL2 queue

using namespace std;

void benchmark(int num_threads, int ops_per_thread) {
    BoundedQueue<int> q(1000);

    auto start = chrono::high_resolution_clock::now();

    vector<thread> threads;

    for (int t = 0; t < num_threads; t++) {
        threads.emplace_back([&]() {
            for (int i = 0; i < ops_per_thread; i++) {
                if (i % 2 == 0)
                    q.try_enq(i);
                else
                    q.try_deq();
            }
        });
    }

    for (auto& th : threads) th.join();

    auto end = chrono::high_resolution_clock::now();

    double seconds = chrono::duration<double>(end - start).count();
    double total_ops = num_threads * ops_per_thread;

    cout << num_threads << "," << (total_ops / seconds) << endl;
}

int main() {
    for (int t = 1; t <= 8; t *= 2) {
        benchmark(t, 100000);
    }
}