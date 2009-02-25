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

TRY_TEST(array_allocation) {
  Runtime runtime;
  try runtime.initialize();
  protector<2> protect(runtime.refs());
  static const word size = 5;
  try alloc ref<Array> array = runtime.gc_safe().new_array(size);
  try alloc ref<String> str = runtime.gc_safe().new_string("blaf");
  for (word i = 0; i < size; i++) {
    ref<Value> value;
    if (i % 2 == 0) value = array;
    else value = str;
    array->set(i, *value);
  }
  Array *old_array = *array;
  runtime.heap().collect_garbage();
  assert *array != old_array;
  for (word i = 0; i < size; i++) {
    ref<Value> expected;
    if (i % 2 == 0) expected = array;
    else expected = str;
    assert array->get(i) == *expected;
  }
  return Success::make();
}

TRY_TEST(trigger_collection) {
  static const word kSaved = 10;
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
      for (word k = 0; k <= i; k++) {
        string_stream stream;
        stream.add("str-%-0", vargs(k));
        assert (saved[k]->equals(stream.raw_string()));
      }
    }
  }
  return Success::make();
}

class Allocator {
public:
  boole initialize() { return runtime().initialize(); }
  Object *allocate(word type);
  static const word kTypeCount = 1;
  Runtime &runtime() { return runtime_; }
private:
  Runtime runtime_;
};

Object *Allocator::allocate(word type) {
  assert 0 <= type;
  assert type < kTypeCount;
  GcSafe &gc_safe = runtime().gc_safe();
  switch (type) {
  case 0:
    return gc_safe.new_string("squid can't win").value();
  default:
    assert false;
    return NULL;
  }
}

TRY_TEST(iteration) {
  static const word kIterations = 3 * Allocator::kTypeCount;
  Allocator allocator;
  try allocator.initialize();
  protector<kIterations> protect(allocator.runtime().refs());
  embed_array<Object*, kIterations> objs;
  for (word i = 0; i < kIterations; i++) {
    ref<Object> obj = protect(allocator.allocate(i % Allocator::kTypeCount));
    objs[i] = *obj;
    SpaceIterator iter(allocator.runtime().heap().space());
    for (word j = 0; j <= i; j++) {
      assert iter.has_next();
      Object *current = iter.next();
      while (current < objs[j] && iter.has_next())
        current = iter.next();
      assert current == objs[j];
    }
    assert !iter.has_next();
  }
  return Success::make();
}
