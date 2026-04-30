#include "coarse_stack.h" // your TL2 queue
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

using namespace std;

void benchmark(int threads, int ops) {
  StackSTM<int> st;

  vector<thread> ts;

  auto start = chrono::high_resolution_clock::now();

  for (int t = 0; t < threads; t++) {
    ts.emplace_back([&]() {
      for (int itr = 0; itr < ops; ++itr) {
        if (t & 1) {
          st.push(2);
          st.push(4);
        } else {
          st.try_pop();
          st.try_pop();
        }
      }
    });
  }

  for (auto &th : ts)
    th.join();

  auto end = chrono::high_resolution_clock::now();
  double sec = chrono::duration<double>(end - start).count();

  cout << threads << "," << fixed << (threads * ops / sec) << endl;
}

int main() {
  // number of operations per thread
  int ops = 100000;

  // thread counts you want to benchmark
  vector<int> thread_counts = {1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 24, 30};

  // CSV header (useful for plotting later)
  cout << "threads,throughput" << endl;

  for (int t : thread_counts) {
    benchmark(t, ops);
  }

  return 0;
}
