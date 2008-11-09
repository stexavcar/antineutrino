#include "cctest/nunit-inl.h"
#include "runtime/runtime-inl.h"
#include "values/values-inl.pp.h"

using namespace neutrino;

TEST(hash_map_simple) {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  HashMap *dict = heap.new_hash_map().value();
  // { }
  @check is<Nothing>(dict->get(heap.new_string("flab").value()));
  @check is<Nothing>(dict->get(heap.new_string("foo").value()));
  @check is<Nothing>(dict->get(Smi::from_int(18)));
  @check is<Nothing>(dict->get(heap.new_string("by").value()));
  // { "flab": 202 }
  dict->set(heap, heap.new_string("flab").value(), Smi::from_int(202));
  @check cast<Smi>(dict->get(heap.new_string("flab").value()))->value() == 202;
  @check is<Nothing>(dict->get(heap.new_string("foo").value()));
  @check is<Nothing>(dict->get(Smi::from_int(18)));
  @check is<Nothing>(dict->get(heap.new_string("by").value()));
  // { "flab": 202, "foo": "123" }
  dict->set(heap, heap.new_string("foo").value(), heap.new_string("123").value());
  @check cast<Smi>(cast<Value>(dict->get(heap.new_string("flab").value())))->value() == 202;
  CHECK(heap.new_string("123").value()->equals(cast<Value>(dict->get(heap.new_string("foo").value()))));
  @check is<Nothing>(dict->get(Smi::from_int(18)));
  @check is<Nothing>(dict->get(heap.new_string("by").value()));
  // { "flab": 202, "foo": "123", 18: 23 }
  dict->set(heap, Smi::from_int(18), Smi::from_int(23));
  @check cast<Smi>(dict->get(heap.new_string("flab").value()))->value() == 202;
  CHECK(heap.new_string("123").value()->equals(cast<Value>(dict->get(heap.new_string("foo").value()))));
  CHECK(Smi::from_int(23)->equals(cast<Value>(dict->get(Smi::from_int(18)))));
  @check is<Nothing>(dict->get(heap.new_string("by").value()));
  // { "flab": 202, "foo": "123", 18: 23, "by": "flab" }
  dict->set(heap, heap.new_string("by").value(), heap.new_string("flab").value());
  @check cast<Smi>(cast<Value>(dict->get(heap.new_string("flab").value())))->value() == 202;
  CHECK(heap.new_string("123").value()->equals(cast<Value>(dict->get(heap.new_string("foo").value()))));
  CHECK(Smi::from_int(23)->equals(cast<Value>(dict->get(Smi::from_int(18)))));
  CHECK(heap.new_string("flab").value()->equals(cast<Value>(dict->get(heap.new_string("by").value()))));
}

TEST(hash_map_iterator) {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  HashMap *dict = heap.new_hash_map().value();
  const int kCount = 100;
  for (int i = 0; i < kCount; i++)
    dict->set(heap, Smi::from_int(i), Smi::from_int(i + 7));
  HashMap::Iterator iter(dict);
  HashMap::Iterator::Entry entry;
  bool visited[kCount];
  for (int i = 0; i < kCount; i++)
    visited[i] = false;
  while (iter.next(&entry)) {
    word i = cast<Smi>(entry.key)->value();
    @check i + 7 == cast<Smi>(entry.value)->value();
    visited[i] = true;
  }
  for (int i = 0; i < kCount; i++)
    CHECK(visited[i]);
}
