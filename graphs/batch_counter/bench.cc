#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include "tl2/tl2.h"

using namespace std;
using namespace tl2;

void benchmark(int threads, int iters) {
    TVar<int> counter(0);

    auto start = chrono::high_resolution_clock::now();

    vector<thread> ts;

    for (int t = 0; t < threads; t++) {
        ts.emplace_back([&]() {
            for (int i = 0; i < iters; i++) {
                atomically([&]() {
                    int v = (int)counter;
                    for (int k = 0; k < 1000; k++)
                        v++;
                    counter = v;
                });
            }
        });
    }

    for (auto &th : ts) th.join();

    auto end = chrono::high_resolution_clock::now();
    double sec = chrono::duration<double>(end - start).count();

    cout << threads << "," << (threads * iters / sec) << endl;
}

int main() {
    cout << "threads,throughput\n";
    for (int t : {1,2,4,8})
        benchmark(t, 100000);
}