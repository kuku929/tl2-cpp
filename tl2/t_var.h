#pragma once
#include <cassert>
#include <iostream>
#include "types.h"
#include "read_set.h"
#include "write_set.h"
#include "hash_table.h"
#include <thread>

// TODO: Any way to detect invalid get/set at compile time?
thread_local bool in_transaction = false;

// Assumption: a variable is only an int
// Can I map the full memory space though?
struct TVar {
public:
    TVar(uint data) : m_data(data) {}

    uint get() {
        assert(in_transaction);
        const auto address = &m_data;
        /*see if address exists in read-set*/ {
            if(read_set.find({ address, m_data }) == read_set.end()) {
                read_set.insert({ address, hashtbl[address].get_version() });
            }
        }
        /*find the value*/ {
            const auto itr = write_set.find({ address, m_data });
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
