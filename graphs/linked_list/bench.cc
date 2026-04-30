#include "coarse_list.h"
#include "fine_list.h"
#include "lockfree_list.h"
#include "zoo/linked_list.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace std;

namespace {

struct Workload {
  string name;
  int contains_pct;
  int add_pct;
  int remove_pct;
};

const vector<Workload> kWorkloads = {
    {"read_heavy", 90, 5, 5},
    {"balanced", 34, 33, 33},
    {"write_heavy", 20, 40, 40},
};

struct Rng {
  std::mt19937 gen;
  std::uniform_int_distribution<int> op_dist;
  std::uniform_int_distribution<int> key_dist;
  Rng(int seed, int key_range)
      : gen(seed), op_dist(0, 99), key_dist(0, key_range - 1) {}
};

template <typename List>
void prefill(List &list, int count) {
  for (int i = 0; i < count; ++i) {
    list.add(i);
  }
}

template <typename List>
double run_benchmark(const string &list_name, const Workload &workload, int threads,
                     int ops_per_thread, int key_range, int prefill_count) {
  List list;
  prefill(list, prefill_count);

  vector<thread> ts;
  ts.reserve(threads);

  auto start = chrono::high_resolution_clock::now();

  for (int t = 0; t < threads; ++t) {
    ts.emplace_back([&, t]() {
      Rng rng(1234 + t * 17, key_range);
      for (int i = 0; i < ops_per_thread; ++i) {
        int op = rng.op_dist(rng.gen);
        int key = rng.key_dist(rng.gen);
        if (op < workload.contains_pct) {
          list.contains(key);
        } else if (op < workload.contains_pct + workload.add_pct) {
          list.add(key);
        } else {
          list.remove(key);
        }
      }
    });
  }

  for (auto &th : ts) {
    th.join();
  }

  auto end = chrono::high_resolution_clock::now();
  double sec = chrono::duration<double>(end - start).count();
  double total_ops = static_cast<double>(threads) * ops_per_thread;

      double throughput = total_ops / sec;
      cout << list_name << ',' << workload.name << ',' << threads << ',' << fixed
        << setprecision(2) << throughput << '\n';
      return throughput;
}

} // namespace

int main() {

  const int ops_per_thread = 10000;
  const int key_range = 1024;
  const int prefill_count = 512;
  const string csv_path = "graphs/linked_list/results.csv";

  ofstream csv(csv_path);
  csv << "list,workload,threads,throughput" << '\n';
  cout << "list,workload,threads,throughput" << '\n';

  for (const auto &workload : kWorkloads) {
    for (int t = 1; t <= 1; t *= 2) {
        double stm = run_benchmark<zoo::ConcurrentLinkedList<int>>(
          "stm", workload, t, ops_per_thread, key_range, prefill_count);
        csv << "stm," << workload.name << ',' << t << ',' << fixed
          << setprecision(2) << stm << '\n';

        // double coarse = run_benchmark<graphs::linked_list::CoarseList<int>>(
        //   "coarse", workload, t, ops_per_thread, key_range, prefill_count);
        // csv << "coarse," << workload.name << ',' << t << ',' << fixed
        //   << setprecision(2) << coarse << '\n';

        // double fine = run_benchmark<graphs::linked_list::FineList<int>>(
        //   "fine", workload, t, ops_per_thread, key_range, prefill_count);
        // csv << "fine," << workload.name << ',' << t << ',' << fixed
        //   << setprecision(2) << fine << '\n';

        // double lockfree = run_benchmark<graphs::linked_list::LockFreeList<int>>(
        //   "lockfree", workload, t, ops_per_thread, key_range, prefill_count);
        // csv << "lockfree," << workload.name << ',' << t << ',' << fixed
        //   << setprecision(2) << lockfree << '\n';
    }
  }

  return 0;
}
