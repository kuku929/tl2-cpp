#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "bounded_queue.h"  // your TL2 queue

using namespace std;


void benchmark_batch(int threads, int ops) {
    BoundedQueue<int> q(1024);

    auto start = chrono::high_resolution_clock::now();

    vector<thread> ts;

    for (int t = 0; t < threads; t++) {
        ts.emplace_back([&]() {
            for (int i = 0; i < ops; i++) {
                atomically([&]() {
                    for (int k = 0; k < 10; k++) {
                        q.try_enq(i + k);
                        q.try_deq();
                    }
                });
            }
        });
    }

    for (auto &th : ts) th.join();

    auto end = chrono::high_resolution_clock::now();
    double sec = chrono::duration<double>(end - start).count();

    cout << threads << "," << (threads * ops / sec) << endl;
}

