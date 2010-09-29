#include "cctest/nunit-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.pp.h"

using namespace neutrino;

TEST(integers) {
  for (int i = -100; i < 100; i++) {
    Smi *value = Smi::from_int(i);
    @check value->value() == i;
  }
}

TEST(simple_string_operations) {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  String *str = heap.new_string("fop").value();
  @check is<String>(str);
  @check str->length() == 3;
  @check str->get(0) == 'f';
  @check str->get(1) == 'o';
  @check str->get(2) == 'p';
}

TEST(to_string) {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  CHECK(Smi::from_int(123)->to_string() == "123");
  CHECK(heap.new_string("knallert").value()->to_string() == "\"knallert\"");
  CHECK(Smi::from_int(123)->to_short_string() == "123");
  CHECK(heap.new_string("knallert").value()->to_short_string() == "\"knallert\"");
  CHECK(Nothing::make()->to_string() == "@<nothing>");
  Tuple *tuple = heap.new_tuple(3).value();
  tuple->set(0, Smi::from_int(123));
  tuple->set(1, Smi::from_int(456));
  tuple->set(2, Smi::from_int(789));
  CHECK(tuple->to_string() == "[123, 456, 789]");
  tuple->set(1, tuple);
  CHECK(tuple->to_string() == "[123, #<tuple>, 789]");
}

// Checks that the root indices are unique
TEST(roots) {
  switch (0) {
#define DECLARE_ROOT_CASE(n, Type, name, Name, allocator) case n:
eRoots(DECLARE_ROOT_CASE)
#undef DECLARE_ROOT_CASE
    return;
  }
}

TEST(cast_failure) {
#ifdef DEBUG
  Value *value = Smi::from_int(0);
  CHECK_ABORTS(cnCastError, cast<Tuple>(value));
#endif
}