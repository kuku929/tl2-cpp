#pragma once
#include <set>
#include <tuple>
#include "types.h"

using ReadOp = std::tuple<uint *, version_t>;

struct ReadSetCompare {
    bool operator () (const ReadOp& a, const ReadOp& b) const {
        return std::get<addr_t>(a) < std::get<addr_t>(b);
    }
};
struct ReadSet : public std::set<ReadOp, ReadSetCompare> {
public:
    ReadSet() : std::set<ReadOp, ReadSetCompare>() {}
} thread_local read_set;
