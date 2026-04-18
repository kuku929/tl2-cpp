#pragma once
#include <mutex> // for now
#include "types.h"

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
