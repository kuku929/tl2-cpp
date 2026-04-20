#pragma once
#include "hash_table.h"
#include "t_var.h" // for in_transaction
#include "read_set.h"
#include "write_set.h"


namespace STM {
    static void commit(version_t write_version) {
        for(const auto& op : write_set) {
            *op.address = op.val;
            hashtbl[op.address].unsafe_set_version(write_version);
        }
    }

    static bool try_commit(const auto read_version) {
        auto guard = make_lock_guard(write_set.begin(), write_set.end(), [](const auto& op) -> VersionLock& {
            return hashtbl[op.address];
        });
        const auto write_version = global_clock.incr_version();
        if(write_version == read_version + 1) {
            // no other thread has made changes commit
            commit(write_version);
            return true;
        }
        for(const auto& op : read_set) {
            if(hashtbl[std::get<addr_t>(op)].unsafe_get_version() > read_version) {
                return false;
            }
        }
        commit(write_version);
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
