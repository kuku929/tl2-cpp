#include "server.h"
#include "examples/bank/transaction.h"
#include "stm.h"
#include <chrono>
#include <random>
#include <algorithm>
#include <thread>

const int MAX_THREADS = 2;

void Server::spin() {
    // in one iteration we spawn some
    // number of threads and await.
    // for now only withdraw supported.
    int num_transactions = random_int(2 * MAX_THREADS);
    for(int _ = 0; _ < num_transactions; ++_) {
        auto transaction = construct_withdraw();
        if(running.size() == MAX_THREADS) {
            std::for_each(running.begin(), running.end(), [](std::thread& t) {
                t.join();
            });
            running.clear();
        }
        running.emplace_back(std::thread(
            [transaction]() {
                transaction.execute();
            }
        ));
        std::this_thread::sleep_for(std::chrono::milliseconds(random_int(100)));
    }
    std::for_each(running.begin(), running.end(), [](std::thread& t) {
        t.join();
    });
}

std::string Server::random_name() {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, names.size() - 1);
    return names[dist(rng)];
}

int Server::random_int(int max, int min) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(min, max);
    return dist(rng);
}

Withdraw Server::construct_withdraw() {
    return Withdraw(random_name(), random_int(1000));
}