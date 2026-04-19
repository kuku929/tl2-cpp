#pragma once
#include <cassert>
#include "types.h"
#include "read_set.h"
#include "write_set.h"
#include "hash_table.h"

namespace STM {
    struct TxContext;
}
/* TVar::get() and TVar::set() now require a STM::TxContext parameter in t_var.h.
STM::TxContext cannot be directly constructed by users, because its constructor is private in stm.h.
Only STM::atomically() is a friend and can create that token in stm.h.
So code outside a transaction cannot call get/set (it has no valid token), and this fails at compile time.
So this replaces runtime assert(in_transaction) with compile-time API gating.
*/


// Assumption: a variable is only an int
// Can I map the full memory space though?
struct TVar {
public:
    TVar(uint data) : m_data(data) {}

    uint get(const STM::TxContext&) {
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

    void set(const STM::TxContext&, uint val) {
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
