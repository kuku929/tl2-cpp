#pragma once
#include <atomic>
#include <thread>
#include <unordered_set>
#include <vector>
#include "types.h"

struct VersionLock {
public:
    version_t get_version() const {
        while (true) {
            const auto s = m_state.load(std::memory_order_acquire);
            if ((s & kLocked) == 0) return unpack_version(s);
            std::this_thread::yield();
        }
    }

    version_t unsafe_get_version() const {
        return unpack_version(m_state.load(std::memory_order_relaxed));
    }

    version_t incr_version() {
        const auto new_state =
            m_state.fetch_add(kVersionInc, std::memory_order_acq_rel) + kVersionInc;
        return unpack_version(new_state);
    }

    void set_version(version_t v) {
        while (true) {
            auto s = m_state.load(std::memory_order_acquire);
            if (s & kLocked) { std::this_thread::yield(); continue; }
            if (m_state.compare_exchange_weak(
                    s, pack(v, false),
                    std::memory_order_release,
                    std::memory_order_acquire)) return;
        }
    }

    void unsafe_set_version(version_t v) {
        const auto s = m_state.load(std::memory_order_relaxed);
        const bool locked = (s & kLocked) != 0;
        m_state.store(pack(v, locked), std::memory_order_release);
    }

    void lock() {
        while (true) {
            auto s = m_state.load(std::memory_order_acquire);
            if (s & kLocked) { std::this_thread::yield(); continue; }
            if (m_state.compare_exchange_weak(
                    s, s | kLocked,
                    std::memory_order_acquire,
                    std::memory_order_relaxed)) return;
        }
    }

    void unlock() {
        m_state.fetch_and(~kLocked, std::memory_order_release);
    }
private:
    static constexpr version_t kLocked = 1;
    static constexpr version_t kVersionInc = 2;

    static constexpr version_t pack(version_t v, bool locked) {
        return (v << 1) | (locked ? 1 : 0);
    }

    static constexpr version_t unpack_version(version_t s) {
        return s >> 1;
    }

    std::atomic<version_t> m_state{0};
} static global_clock;

template<typename It, typename LockSelector>
struct LockGuard {
public:
    LockGuard(It begin, It end, LockSelector lock_selector) {
        std::unordered_set<VersionLock*> seen;
        for(auto it = begin; it != end; ++it) {
            VersionLock* lock = &lock_selector(*it);
            if(seen.insert(lock).second) {
                lock->lock();
                m_locked.push_back(lock);
            }
        }
    }

    ~LockGuard() {
        for(auto it = m_locked.rbegin(); it != m_locked.rend(); ++it) {
            (*it)->unlock();
        }
    }

    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
    LockGuard(LockGuard&&) = delete;
    LockGuard& operator=(LockGuard&&) = delete;

private:
    std::vector<VersionLock*> m_locked;
};

template<typename It, typename LockSelector>
auto make_lock_guard(It begin, It end, LockSelector lock_selector) {
    return LockGuard<It, LockSelector>(begin, end, lock_selector);
}
