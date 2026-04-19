#pragma once
#include "hash_table.h"
#include "read_set.h"
#include "write_set.h"


namespace STM {
    struct TxContext {
    private:
        struct PrivateTag {};
        explicit TxContext(PrivateTag) {}
        template<typename Transaction> friend void atomically(Transaction t);
    };
    /* TVar::get() and TVar::set() now require a STM::TxContext parameter in t_var.h.
    STM::TxContext cannot be directly constructed by users, because its constructor is private in stm.h.
    Only STM::atomically() is a friend and can create that token in stm.h.
    So code outside a transaction cannot call get/set (it has no valid token), and this fails at compile time.
    So this replaces runtime assert(in_transaction) with compile-time API gating.
    */

    static bool lock_write_set() {
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
            auto tx = TxContext(TxContext::PrivateTag{});
            t(tx);
            if(try_commit(read_version)) break;
        }
    }
};
