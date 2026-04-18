#pragma once
#include <set>
#include "types.h"

struct WriteOp {
    uint *address;
    uint val;
};

struct WriteSetCompare {
    bool operator () (const WriteOp& a, const WriteOp& b) const {
        return a.address < b.address;
    }
};
struct WriteSet : public std::set<WriteOp, WriteSetCompare> {
    WriteSet() : std::set<WriteOp, WriteSetCompare>() {}
} thread_local write_set;