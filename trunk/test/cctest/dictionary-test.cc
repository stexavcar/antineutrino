#include "runtime/runtime-inl.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::dictionary_simple() {
  Runtime runtime;
  runtime.initialize();
  Heap &heap = runtime.heap();
  Dictionary *dict = cast<Dictionary>(heap.new_dictionary());
  // { }
  CHECK_IS(Nothing, dict->get(cast<Value>(heap.new_string("flab"))));
  CHECK_IS(Nothing, dict->get(cast<Value>(heap.new_string("foo"))));
  CHECK_IS(Nothing, dict->get(cast<Value>(Smi::from_int(18))));
  CHECK_IS(Nothing, dict->get(cast<Value>(heap.new_string("by"))));
  // { "flab": 202 }
  dict->set(heap, cast<Value>(heap.new_string("flab")), Smi::from_int(202));
  CHECK_EQ(202, cast<Smi>(dict->get(cast<Value>(heap.new_string("flab"))))->value());
  CHECK_IS(Nothing, dict->get(cast<Value>(heap.new_string("foo"))));
  CHECK_IS(Nothing, dict->get(Smi::from_int(18)));
  CHECK_IS(Nothing, dict->get(cast<Value>(heap.new_string("by"))));
  // { "flab": 202, "foo": "123" }
  dict->set(heap, cast<Value>(heap.new_string("foo")), cast<Value>(heap.new_string("123")));
  CHECK_EQ(202, cast<Smi>(cast<Value>(dict->get(cast<Value>(heap.new_string("flab")))))->value());
  CHECK(cast<Value>(heap.new_string("123"))->equals(cast<Value>(dict->get(cast<Value>(heap.new_string("foo"))))));
  CHECK_IS(Nothing, dict->get(Smi::from_int(18)));
  CHECK_IS(Nothing, dict->get(cast<Value>(heap.new_string("by"))));
  // { "flab": 202, "foo": "123", 18: 23 }
  dict->set(heap, Smi::from_int(18), Smi::from_int(23));
  CHECK_EQ(202, cast<Smi>(dict->get(cast<Value>(heap.new_string("flab"))))->value());
  CHECK(cast<Value>(heap.new_string("123"))->equals(cast<Value>(dict->get(cast<Value>(heap.new_string("foo"))))));
  CHECK(Smi::from_int(23)->equals(cast<Value>(dict->get(Smi::from_int(18)))));
  CHECK_IS(Nothing, dict->get(cast<Value>(heap.new_string("by"))));
  // { "flab": 202, "foo": "123", 18: 23, "by": "flab" }
  dict->set(heap, cast<Value>(heap.new_string("by")), cast<Value>(heap.new_string("flab")));
  CHECK_EQ(202, cast<Smi>(cast<Value>(dict->get(cast<Value>(heap.new_string("flab")))))->value());
  CHECK(cast<Value>(heap.new_string("123"))->equals(cast<Value>(dict->get(cast<Value>(heap.new_string("foo"))))));
  CHECK(Smi::from_int(23)->equals(cast<Value>(dict->get(Smi::from_int(18)))));
  CHECK(cast<Value>(heap.new_string("flab"))->equals(cast<Value>(dict->get(cast<Value>(heap.new_string("by"))))));
}

void Test::dictionary_iterator() {
  Runtime runtime;
  runtime.initialize();
  Heap &heap = runtime.heap();
  Dictionary *dict = cast<Dictionary>(heap.new_dictionary());
  const int kCount = 100;
  for (int i = 0; i < kCount; i++)
    dict->set(heap, Smi::from_int(i), Smi::from_int(i + 7));
  Dictionary::Iterator iter(dict);
  Dictionary::Iterator::Entry entry;
  bool visited[kCount];
  for (int i = 0; i < kCount; i++)
    visited[i] = false;
  while (iter.next(&entry)) {
    word i = cast<Smi>(entry.key)->value();
    CHECK_EQ(cast<Smi>(entry.value)->value(), i + 7);
    visited[i] = true;
  }
  for (int i = 0; i < kCount; i++)
    CHECK(visited[i]);
}
