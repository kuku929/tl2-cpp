#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <optional>
#include "tl2/tl2.h"

using namespace std;
using namespace tl2;

template<typename T>
class STMQueue {
public:
    STMQueue(size_t capacity)
        : cap(capacity),
          buffer(capacity, TVar<T>(T())),
          head(0),
          tail(0) {}

    bool try_enqueue(const T& x) {
        bool success = false;
        atomically([&]() {
            size_t h = (size_t)head;
            size_t t = (size_t)tail;
            if (t - h == cap) return;
            buffer[t % cap] = x;
            tail = t + 1;
            success = true;
        });
        return success;
    }

    optional<T> try_dequeue() {
        optional<T> result = nullopt;
        atomically([&]() {
            size_t h = (size_t)head;
            size_t t = (size_t)tail;
            if (t == h) return;
            result = (T)buffer[h % cap];
            head = h + 1;
        });
        return result;
    }

private:
    size_t cap;
    vector<TVar<T>> buffer;
    TVar<size_t> head, tail;
};

void print_header(const string& name) {
    cout << "\n# " << name << "\n";
    cout << "threads,throughput\n";
}

// ---------------- LOW CONTENTION ----------------
void bench_low_contention(int threads, int ops) {
    vector<STMQueue<int>> qs;
    for (int i = 0; i < threads; i++)
        qs.emplace_back(1024);

    auto start = chrono::high_resolution_clock::now();

    vector<thread> ts;
    for (int t = 0; t < threads; t++) {
        ts.emplace_back([&, t]() {
            int dummy = 0;
            for (int i = 0; i < ops; i++) {
                if (i % 2 == 0)
                    qs[t].try_enqueue(i);
                else {
                    auto v = qs[t].try_dequeue();
                    if (v) dummy += *v;
                }
            }
        });
    }

    for (auto &th : ts) th.join();

    auto end = chrono::high_resolution_clock::now();
    double sec = chrono::duration<double>(end - start).count();

    cout << threads << "," << (threads * ops / sec) << endl;
}

// ---------------- READ HEAVY ----------------
void bench_read_heavy(int threads, int ops) {
    STMQueue<int> q(1024);

    auto start = chrono::high_resolution_clock::now();

    vector<thread> ts;
    for (int t = 0; t < threads; t++) {
        ts.emplace_back([&]() {
            int dummy = 0;
            for (int i = 0; i < ops; i++) {
                if (i % 10 == 0)
                    q.try_enqueue(i);
                else {
                    auto v = q.try_dequeue();
                    if (v) dummy += *v;
                }
            }
        });
    }

    for (auto &th : ts) th.join();

    auto end = chrono::high_resolution_clock::now();
    double sec = chrono::duration<double>(end - start).count();

    cout << threads << "," << (threads * ops / sec) << endl;
}

// ---------------- BATCH ----------------
void bench_batch(int threads, int ops) {
    STMQueue<int> q(1024);

    auto start = chrono::high_resolution_clock::now();

    vector<thread> ts;
    for (int t = 0; t < threads; t++) {
        ts.emplace_back([&]() {
            for (int i = 0; i < ops; i++) {
                atomically([&]() {
                    for (int k = 0; k < 10; k++) {
                        q.try_enqueue(i + k);
                        q.try_dequeue();
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

int main() {
    vector<int> thread_counts = {1,2,4,8};

    print_header("low_contention");
    for (int t : thread_counts) bench_low_contention(t, 100000);

    print_header("read_heavy");
    for (int t : thread_counts) bench_read_heavy(t, 100000);

    print_header("batch");
    for (int t : thread_counts) bench_batch(t, 10000);

    return 0;
}