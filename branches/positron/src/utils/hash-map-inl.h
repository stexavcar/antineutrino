#ifndef _UTILS_HASH_MAP_INL
#define _UTILS_HASH_MAP_INL

#include "utils/check.h"
#include "utils/hash-map.h"
#include "utils/smart-ptrs-inl.h"

namespace neutrino {

template <typename K, typename V, class A>
hash_map<K, V, A>::hash_map(A allocator)
  : entry_count_(0)
  , allocator_(allocator) {
  entries_.set(vector< hash_map_entry<K, V> >::allocate(allocator, kInitialCapacity));
}

template <typename K, typename V, class A>
hash_map<K, V, A>::~hash_map() { }

template <typename K, typename V>
void hash_map_entry<K, V>::grab(const K &key, const V &value, uword hash) {
  assert !is_occupied();
  hash_ = hash;
  key_ = key;
  value_ = value;
  is_occupied_ = true;
}

template <typename K, typename V, class A>
V &hash_map<K, V, A>::put(const K &key, const V &value) {
  if (size() == entries().length())
    extend_capacity(grow_value(size()));
  uword h = hash(key);
  hash_map_entry<K, V> &entry = lookup(key, h);
  if (entry.is_occupied()) {
    assert entry.hash() == h;
    assert entry.key() == key;
    entry.set_value(value);
  } else {
    entry_count_++;
    entry.grab(key, value, h);
  }
  return entry.value();
}

template <typename K, typename V, class A>
bool hash_map<K, V, A>::get(const K &key, V **value_out) {
  hash_map_entry<K, V> &entry = lookup(key, hash(key));
  if (entry.is_occupied()) {
    (*value_out) = &entry.value();
    return true;
  } else {
    return false;
  }
}

template <typename K, typename V, class A>
const V &hash_map<K, V, A>::get(const K &key, const V &if_missing) {
  V *result;
  if (get(key, &result)) {
    return *result;
  } else {
    return if_missing;
  }
}

template <typename K, typename V, class A>
hash_map_entry<K, V> &hash_map<K, V, A>::lookup(const K &key, uword hash) {
  vector< hash_map_entry<K, V> > entries = this->entries();
  word i;
  for (i = hash % entries.length();
       entries[i].is_occupied();
       i = (i + 1) % entries.length()) {
    hash_map_entry<K, V> &current = entries[i];
    if (current.hash() == hash && current.key() == key)
      return current;
  }
  return entries[i];
}

template <typename K, typename V, class A>
void hash_map<K, V, A>::extend_capacity(word new_capacity) {
  assert new_capacity > entries().length();
  own_vector< hash_map_entry<K, V> > old_entries(entries_.release());
  entries_.set(vector< hash_map_entry<K, V> >::allocate(allocator(), new_capacity));
  for (word i = 0; i < old_entries.length(); i++) {
    hash_map_entry<K, V> &old_entry = old_entries[i];
    if (!old_entry.is_occupied()) continue;
    hash_map_entry<K, V> &new_entry = lookup(old_entry.key(), old_entry.hash());
    assert !new_entry.is_occupied();
    new_entry.grab(old_entry.key(), old_entry.value(), old_entry.hash());
  }
}

} // namespace neutrino

#endif // _UTILS_HASH_MAP_INL
