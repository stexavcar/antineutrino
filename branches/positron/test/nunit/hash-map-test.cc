#include "plankton/plankton-inl.h"
#include "runtime/gc-safe-inl.h"
#include "runtime/runtime-inl.h"
#include "utils/hash-map-inl.h"
#include "value/condition-inl.h"
#include "value/value-inl.h"
#include "test-inl.h"

using namespace neutrino;

static void compare_maps(buffer< pair<word, word> > &values, hash_map<word, word> &map) {
  for (word i = 0; i < values.length(); i++) {
    word key = values[i].first();
    word value = 0;
    // Since there may be more than one mapping for a given key in the
    // values list we look up the last value for any given key.
    for (word j = values.length() - 1; j >= 0; j--) {
      if (values[j].first() == key) {
        value = values[j].second();
        break;
      }
    }
    assert map.get(key, value + 1) == value;
  }
}

TEST(hash_map) {
  hash_map<word, word> map;
  PseudoRandom random(77);
  buffer< pair<word, word> > values;
  for (word i = 0; i < 100; i++) {
    word key = random.next();
    word value = random.next();
    map.put(key, value);
    values.append(pair<word, word>(key, value));
    compare_maps(values, map);
  }
}

TEST(different_types) {
  hash_map<string, p::String> map;
  for (word i = 0; i < 100; i++) {
    string_stream stream;
    stream.add("key-%", vargs(i));
    string str = string::dup(stream.raw_string());
    map.put(str, p::String(str.start()));
  }
  p::String empty = p::String("");
  for (word i = 0; i < 100; i++) {
    string_stream stream;
    stream.add("key-%", vargs(i));
    p::String str = map.get(stream.raw_string(), empty);
    assert str == p::String(stream.raw_string().start());
  }
}

TRY_TEST(hash_map_object) {
  Runtime runtime;
  try runtime.initialize();
  protector<1> protect(runtime.refs());
  try alloc ref<HashMap> map = runtime.gc_safe().new_hash_map();
  assert 0 == map->size();
  PseudoRandom random(35);
  for (word i = 0; i < 100; i++) {
    protector<2> protect(runtime.refs());
    string_stream stream;
    stream.add("key-%-value", vargs(i));
    try alloc ref<String> key = runtime.gc_safe().new_string(stream.raw_string());
    ref<TaggedInteger> value = protect(random.next_tagged_integer());
    try runtime.gc_safe().set(map, key, value);
    assert map->size() == (i + 1);
  }
  random.reset(35);
  for (word i = 0; i < 100; i++) {
    protector<1> protect(runtime.refs());
    string_stream stream;
    stream.add("key-%-value", vargs(i));
    try alloc ref<String> key = runtime.gc_safe().new_string(stream.raw_string());
    assert map->get(*key) == random.next_tagged_integer();
  }
  return Success::make();
}
