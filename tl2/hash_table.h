#pragma once
#include <vector>
#include "version_lock.h"
#include "types.h"

constexpr size_t LOCKTABLE_SIZE = 1<<20;

struct LockTable{
private:
    std::vector<VersionLock> table;
    size_t mask;

public:
    explicit LockTable(const size_t size_pow2) : table(size_pow2) , mask(size_pow2 - 1) {}

    //maps address to version lock (hashing)
    inline VersionLock& operator[] (const uint* addr) {
        const auto x = reinterpret_cast<uintptr_t>(const_cast<addr_t>(addr));
        size_t index = (x>>2) & mask;
        return table[index];
    }
};

static LockTable hashtbl(LOCKTABLE_SIZE);