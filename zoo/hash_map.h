#pragma once

#include "zoo/linked_list.h"
#include <array>
#include <optional>
#include <type_traits>
namespace zoo {
template <typename KeyT, typename ItemT, typename HashFunc> class HashMap {
public:
  HashMap(HashFunc f) : m_hash(std::move(f)) { ; }

  static_assert(std::is_default_constructible_v<ItemT>,
                "Item should be default constructible!");

  bool contains(const KeyT &key) {
    if (auto entry = buckets[m_hash(key) % HASH_MOD].get(LLNode(key));
        entry.has_value()) {
      return true;
    }
    return false;
  }

  std::optional<ItemT> get(const KeyT &key) {
    auto &ll = buckets[m_hash(key) % HASH_MOD];
    std::optional<ItemT> ret{std::nullopt};
    if (auto entry = ll.get(LLNode(key)); entry.has_value()) {
      ret = std::optional<ItemT>(entry->item);
    }
    return ret;
  }

  void set(const KeyT &key, const ItemT &item) {
    auto &ll = buckets[m_hash(key) % HASH_MOD];
    ll.update(LLNode(key), LLNode(key, item));
  }

private:
  static constexpr int HASH_MOD = 1024;
  HashFunc m_hash;
  struct LLNode {
    LLNode(KeyT key) : m_key(key) {}
    LLNode(KeyT key, ItemT item) : m_key(key), item(std::move(item)) {}
    bool operator==(const LLNode &other) const { return m_key == other.m_key; }
    KeyT m_key;
    ItemT item;
  };
  std::array<LinkedList<LLNode>, HASH_MOD> buckets;
};
} // namespace zoo