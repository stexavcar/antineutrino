#include "plankton/plankton-inl.h"
#include "runtime/gc-safe-inl.h"
#include "runtime/heap-inl.h"
#include "runtime/runtime-inl.h"
#include "value/condition-inl.h"
#include "test-inl.h"

using namespace neutrino;

TRY_TEST(simple_allocation) {
  Runtime runtime;
  try runtime.initialize();
  string c_str = "hopla";
  try alloc String *raw_str = runtime.heap().new_string(c_str);
  assert raw_str->length() == c_str.length();
  for (word i = 0; i < c_str.length(); i++)
    assert raw_str->chars()[i] == static_cast<code_point>(c_str[i]);
  protector<1> protect(runtime.refs());
  ref<String> str = protect(raw_str);
  assert str->length() == c_str.length();
  try runtime.heap().collect_garbage();
  assert *str != raw_str;
  assert str->length() == c_str.length();
  for (word i = 0; i < c_str.length(); i++)
    assert str->chars()[i] == static_cast<code_point>(c_str[i]);
  return Success::make();
}

TRY_TEST(trigger_collection) {
  static const word kSaved = 100;
  static const word kSteps = 1000;
  Runtime runtime;
  try runtime.initialize();
  protector<kSaved> outer_protect(runtime.refs());
  embed_array<ref<String>, kSaved> saved;
  for (word i = 0; i < kSaved; i++) {
    for (word j = 0; j < kSteps; j++) {
      string_stream stream;
      stream.add("str-%-%", vargs(i, j));
      protector<1> protect(runtime.refs());
      try alloc ref<String> str = runtime.gc_safe().new_string(stream.raw_string());
      if (j == 0) {
        saved[i] = outer_protect(*str);
      }
    }
  }
  return Success::make();
}
