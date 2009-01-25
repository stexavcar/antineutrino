#ifndef _UTILS_HASH_MAP
#define _UTILS_HASH_MAP

#include "utils/array.h"
#include "utils/smart-ptrs.h"

namespace neutrino {

template <typename K, typename V>
class hash_map_entry {
public:
  hash_map_entry() : is_occupied_(false) { }
  bool is_occupied() { return is_occupied_; }
  uword hash() { return hash_; }
  K &key() { return key_; }
  V &value() { return value_; }
  void set_value(const V &value) { value_ = value; }
  void grab(const K &key, const V &value, uword hash);
private:
  uword hash_;
  K key_;
  V value_;
  bool is_occupied_;
};

template < typename K, typename V, class A = new_delete_array_allocator< hash_map_entry<K, V> > >
class hash_map : public nocopy {
public:
  hash_map(A allocator = A());
  ~hash_map();
  V &put(const K &key, const V &value);
  bool get(const K &key, V **value_out);
  const V &get(const K &key, const V &if_missing);
  hash_map_entry<K, V> &lookup(const K &key, uword hash);
  word size() { return entry_count_; }
private:
  A &allocator() { return allocator_; }
  static const word kInitialCapacity = 8;
  vector< hash_map_entry<K, V> > entries() { return entries_.as_vector(); }
  void extend_capacity(word new_capacity);
  own_vector< hash_map_entry<K, V> > entries_;
  word entry_count_;
  A allocator_;
};

template <typename T>
static uword hash(const T &v);

template <>
static inline uword hash<word>(const word &i) {
  return static_cast<uword>(i);
}

} // namespace neutrino

#endif // _UTILS_HASH_MAP
