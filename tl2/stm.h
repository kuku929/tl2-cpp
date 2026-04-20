#pragma once
#include "hash_table.h"
#include "t_var.h" // for in_transaction
#include "read_set.h"
#include "write_set.h"


namespace STM {
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
        lock_write_set();
        const auto write_version = global_clock.incr_version();
        if(write_version == read_version + 1) {
            // no other thread has made changes commit
            commit(write_version);
            unlock_write_set();
            return true;
        }
        for(const auto& op : read_set) {
            if(hashtbl[std::get<addr_t>(op)].unsafe_get_version() > read_version) {
                unlock_write_set();
                return false;
            }
        }
        commit(write_version);
        unlock_write_set();
        return true;
    };

    // need some machinery to deduce lambda return types and so on...
    template<typename Transaction> static void atomically(Transaction t) {
        while(true) {
            const auto read_version = global_clock.get_version();
            in_transaction = true;
            read_set.clear();
            write_set.clear();
            t();
            in_transaction = false;
            if(try_commit(read_version)) break;
        }
    }
};
