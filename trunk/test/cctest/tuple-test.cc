#include "cctest/nunit-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.pp.h"

using namespace neutrino;

TEST(tuples) {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  Tuple *tuple0 = heap.new_tuple(0).value();
  @check tuple0->length() == 0;
  Tuple *tuple1 = heap.new_tuple(1).value();
  @check tuple1->length() == 1;
  @check is<Null>(tuple1->get(0));
  tuple1->set(0, tuple0);
  CHECK(tuple1->get(0) == tuple0);
  static int kCount = 100;
  Tuple *long_tuple = heap.new_tuple(kCount).value();
  for (int i = 0; i < kCount; i++)
    @check is<Null>(long_tuple->get(i));
  for (int i = 0; i < kCount; i++)
    long_tuple->set(i, Smi::from_int(i - 50));
  for (int i = 0; i < kCount; i++) {
    Value *val = long_tuple->get(i);
    @check cast<Smi>(val)->value() == i - 50;
  }
}

TEST(bounds_check) {
#ifdef DEBUG
  LocalRuntime runtime;
  Tuple *tuple = runtime.heap().new_tuple(10).value();
  CHECK_ABORTS(cnOutOfBounds, tuple->get(10));
#endif
}
