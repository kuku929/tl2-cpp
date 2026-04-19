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
    explicit LockTable(size_t size_pow2) : table(size_pow2) , mask(size_pow2) {}

    //maps address to version lock (hashing)
    inline VersionLock& operator[] (uint32_t* addr) {
        uintptr_t x = reinterpret_cast<uintptr_t>(addr);
        size_t index = (x>>3) & mask;
        return table[index];
    }
};

LockTable hashtbl(LOCKTABLE_SIZE);