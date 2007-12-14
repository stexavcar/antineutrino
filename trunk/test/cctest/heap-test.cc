#include "heap/values-inl.h"
#include "cctest/tests-inl.h"
#include "runtime/runtime.h"

using namespace neutrino;

void Test::integers() {
  for (int i = -100; i < 100; i++) {
    Smi *value = Smi::from_int(i);
    CHECK_EQ(i, value->value());
  }
}

void Test::simple_string_operations() {
  Runtime runtime;
  runtime.initialize();
  Heap &heap = runtime.heap();
  String *str = cast<String>(heap.new_string("fop"));
  CHECK_IS(String, str);
  CHECK_EQ(3, str->length());
  CHECK_EQ('f', str->at(0));
  CHECK_EQ('o', str->at(1));
  CHECK_EQ('p', str->at(2));
}

void Test::to_string() {
  Runtime runtime;
  runtime.initialize();
  Heap &heap = runtime.heap();
  CHECK(Smi::from_int(123)->to_string() == "123");
  CHECK(heap.new_string("knallert")->to_string() == "\"knallert\"");
  CHECK(Smi::from_int(123)->to_short_string() == "123");
  CHECK(heap.new_string("knallert")->to_short_string() == "\"knallert\"");
  CHECK(Nothing::make()->to_string() == "@<nothing>");
  Tuple *tuple = cast<Tuple>(heap.new_tuple(3));
  tuple->set(0, Smi::from_int(123));
  tuple->set(1, Smi::from_int(456));
  tuple->set(2, Smi::from_int(789));
  CHECK(tuple->to_string() == "[123, 456, 789]");
  tuple->set(1, tuple);
  CHECK(tuple->to_string() == "[123, #<tuple>, 789]");
}

void Test::cast_failure() {
#ifdef DEBUG
  Value *value = Smi::from_int(0);
  CHECK_ABORTS(CAST_ERROR, cast<Tuple>(value));
#endif
}
