#pragma once

#include "zoo/linked_list.h"
#include <type_traits>
#include <array>
namespace zoo {
template<typename KeyT, typename ItemT, typename HashFunc>
class HashMap {
public:
    HashMap(HashFunc f) : m_hash(std::move(f)) {;}

    static_assert(std::is_default_constructible_v<ItemT>, "Item should be default constructible!");

    bool contains(const KeyT& key) {
        if(auto entry = buckets[m_hash(key) % HASH_MOD].get(LLNode(key)); entry.has_value()) {
            return true;
        }
        return false;
    }

    ItemT get(const KeyT& key) {
        auto &ll = buckets[m_hash(key) % HASH_MOD];
        ItemT ret{};
        if(auto entry = ll.get(LLNode(key)); entry.has_value()) {
            ret = entry->item;
        } else { ll.add(LLNode(key, ret)); }
        return ret;
    }

    void set(const KeyT& key, const ItemT& item) {
        auto &ll = buckets[m_hash(key) % HASH_MOD];
        ll.update(LLNode(key), LLNode(key, item));
    }

private:
    static constexpr int HASH_MOD = 1024;
    HashFunc m_hash;
    struct LLNode {
        LLNode(KeyT key) : m_key(key) {}
        LLNode(KeyT key, ItemT item) : m_key(key), item(std::move(item)) {}
        bool operator==(const LLNode &other) const {
            return m_key == other.m_key;
        }
        KeyT m_key;
        ItemT item;
    };
    std::array<LinkedList<LLNode>, HASH_MOD> buckets;
};

} // namespace zoo