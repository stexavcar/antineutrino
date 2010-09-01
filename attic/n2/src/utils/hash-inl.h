#ifndef _UTILS_HASH
#define _UTILS_HASH

namespace neutrino {


template <typename Type>
struct HashMapLookup {
  Type *value;
};


template <typename Config>
class HashUtils {
public:
  typedef typename Config::map_t map_t;
  typedef typename Config::raw_table_t raw_table_t;
  typedef typename Config::table_t table_t;
  typedef typename Config::key_t key_t;
  typedef typename Config::value_t value_t;
  typedef typename Config::data_t data_t;
  static inline bool lookup_entry(table_t table, key_t key,
      HashMapLookup<value_t> &lookup);
  static inline bool ensure_entry(map_t map, table_t table, key_t key,
      HashMapLookup<value_t> &lookup, raw_table_t *raw_table,
      data_t data);
};


template <typename C>
bool HashUtils<C>::lookup_entry(table_t table, key_t key,
    HashMapLookup<value_t> &lookup) {
  for (uword i = 0; i < C::table_length(table); i += 2) {
    if (C::keys_equal(key, C::table_get(table, i))) {
      lookup.value = &C::table_get(table, i + 1);
      return true;
    }
  }
  return false;
}


template <typename C>
bool HashUtils<C>::ensure_entry(map_t map, table_t table, key_t key,
    HashMapLookup<value_t> &lookup, raw_table_t *raw_table,
    data_t data) {
  if (lookup_entry(table, key, lookup)) return true;
  uword length = C::table_length(table);
  raw_table_t new_table_val = C::allocate_table(length + 2, data);
  if (C::is_abort_value(new_table_val)) {
    *raw_table = new_table_val;
    return false;
  }
  table_t new_table = C::cast_to_table(new_table_val);
  for (uword i = 0; i < length; i++)
    C::table_set(new_table, i, C::table_get(table, i));
  C::table_set(new_table, length, key);
  lookup.value = &C::table_get(new_table, length + 1);
  C::set_table(map, new_table);
  C::free_table(table);
  return true;
}


} // neutrino

#endif // _UTILS_HASH
