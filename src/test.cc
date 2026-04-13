#include <iostream>
#include <thread>
#include <vector>
#include <array>
#include <cstdint>
#include <cassert>
#include <unordered_map>
#include <tuple>
#include <set>
#include <mutex>
#include <random>
using version_t = uint64_t;
using addr_t    = uint *;

struct WriteOp {
    uint *address;
    uint val;
};

using ReadOp = std::tuple<uint *, version_t>;

struct VersionLock {
public: 
    version_t get_version() {
        const std::lock_guard<std::mutex> lock(m_lock);
        return m_v;
    }

    version_t unsafe_get_version() {
        return m_v;
    }

    version_t incr_version() {
        const std::lock_guard<std::mutex> lock(m_lock);
        m_v++;
        return m_v;
    }

    void set_version(version_t v) {
        const std::lock_guard<std::mutex> lock(m_lock);
        m_v = v;
    }

    void unsafe_set_version(version_t v) {
        m_v = v;
    }

    void lock() {
        m_lock.lock();
    }

    void unlock() {
        m_lock.unlock();
    }
private:
    version_t m_v = 0;
    // TODO: Using a mutex for now. Ideally CAS operation
    std::mutex m_lock;
} static global_clock;

struct ReadSetCompare {
    bool operator () (const ReadOp& a, const ReadOp& b) const {
        return std::get<addr_t>(a) < std::get<addr_t>(b);
    }
};
struct ReadSet : public std::set<ReadOp, ReadSetCompare> {
public:
    ReadSet() : std::set<ReadOp, ReadSetCompare>() {}
} thread_local read_set;


struct WriteSetCompare {
    bool operator () (const WriteOp& a, const WriteOp& b) const {
        return a.address < b.address;
    }
};
struct WriteSet : public std::set<WriteOp, WriteSetCompare> {
    WriteSet() : std::set<WriteOp, WriteSetCompare>() {}
} thread_local write_set;

// TODO: How to specify the hash function here?
// we want something general so that we can test out different hash functions
static std::unordered_map<uint *, VersionLock> hashtbl;

// TODO: Any way to detect invalid get/set at compile time?
thread_local bool in_transaction = false;

namespace STM {
    static bool lock_write_set() {
        // TODO: could be a member function of WriteSet
        for(const auto& op : write_set) {
            // TODO: this should be bounded?
            hashtbl[op.address].lock();
        }
        return true;
    }

    static void unlock_write_set() {
        // TODO: could be a member function of WriteSet
        for(const auto& op : write_set) {
            hashtbl[op.address].unlock();
        }
    }

    static void commit(version_t write_version) {
        for(const auto& op : write_set) {
            *op.address = op.val;
            hashtbl[op.address].unsafe_set_version(write_version);
        }
    }

    static bool try_commit(const auto read_version) {
        const auto write_version = global_clock.incr_version();
        if(write_version == read_version + 1) {
            // no other thread has made changes commit
            commit(write_version);
            return true;
        }
        // std::cout << "acquiring write set" << std::endl;
        lock_write_set();
        for(const auto& op : read_set) {
            if(hashtbl[std::get<addr_t>(op)].unsafe_get_version() > read_version) {
                unlock_write_set();
                return false;
            }
        }
        commit(write_version);
        // std::cout << "unlocking write set" << std::endl;
        unlock_write_set();
        return true;
    };

    // need some machinery to deduce lambda return types and so on...
    template<typename Transaction> static void atomically(Transaction t) {
        while(true) {
            const auto read_version = global_clock.get_version();
            in_transaction = true;
            t();
            in_transaction = false;
            if(try_commit(read_version)) break;
        }
    }
};

// Assumption: a variable is only an int
// Can I map the full memory space though?
struct TVar {
public:
    TVar(uint data) : m_data(data) {}

    uint get() {
        assert(in_transaction);
        /*see if address exists in read-set*/ {
            if(read_set.find({ &m_data, m_data }) == read_set.end()) {
                read_set.insert({ &m_data, hashtbl[reinterpret_cast<addr_t>(m_data)].get_version() });
            }
        }
        /*find the value*/{
            const auto itr = write_set.find({ &m_data, m_data });
            if(itr != write_set.end()) {
                return itr->val;
            }
            return m_data;
        }
    }

    void set(uint val) {
        assert(in_transaction);
        const auto op = WriteOp{
            .address = &m_data, 
            .val = val};
        auto itr = write_set.find(op);
        if(itr != write_set.end()) {
            write_set.erase(itr);
        }
        write_set.insert(op);
    }
private:
    uint m_data;
};

int main(int argc, char *argv[]) {
    // TODO : test cases(a lot of them)
    TVar counter(0);
    std::thread t1([&]() {
        STM::atomically([&]() {
            for(int __i = 0; __i < 100'000; ++__i) {
                uint val = counter.get();
                counter.set(val + 1);
            }
        });
    });
    std::thread t2([&]() {
        STM::atomically([&]() {
            for(int __i = 0; __i < 100'000; ++__i) {
                uint val = counter.get();
                counter.set(val + 1);
            }
        });
    });
    t1.join();
    t2.join();
    STM::atomically([&]() {
        std::cout << counter.get() << std::endl;
    });
}
