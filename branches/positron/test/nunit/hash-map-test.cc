#include "plankton/plankton-inl.h"
#include "utils/hash-map-inl.h"
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
  PseudoRandom random(97);
  for (word i = 0; i < 100; i++) {
    string_stream stream;
    stream.add("key-%", args(i));
    string str = string::dup(stream.raw_c_str());
    map.put(str, p::String(str.start()));
  }
  p::String empty = p::String("");
  for (word i = 0; i < 100; i++) {
    string_stream stream;
    stream.add("key-%", args(i));
    p::String str = map.get(stream.raw_c_str(), empty);
    assert str == p::String(stream.raw_c_str().start());
  }
}
