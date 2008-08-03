#include "cctest/nunit-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.h"

using namespace neutrino;

TEST(tuples) {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  Tuple *tuple0 = heap.new_tuple(0).value();
  CHECK_EQ(0, tuple0->length());
  Tuple *tuple1 = heap.new_tuple(1).value();
  CHECK_EQ(1, tuple1->length());
  CHECK_IS(Null, tuple1->get(0));
  tuple1->set(0, tuple0);
  CHECK(tuple1->get(0) == tuple0);
  static int kCount = 100;
  Tuple *long_tuple = heap.new_tuple(kCount).value();
  for (int i = 0; i < kCount; i++)
    CHECK_IS(Null, long_tuple->get(i));
  for (int i = 0; i < kCount; i++)
    long_tuple->set(i, Smi::from_int(i - 50));
  for (int i = 0; i < kCount; i++) {
    Value *val = long_tuple->get(i);
    CHECK_EQ(i - 50, cast<Smi>(val)->value());
  }
}

TEST(bounds_check) {
#ifdef DEBUG
  LocalRuntime runtime;
  Tuple *tuple = runtime.heap().new_tuple(10).value();
  CHECK_ABORTS(cnOutOfBounds, tuple->get(10));
#endif
}
