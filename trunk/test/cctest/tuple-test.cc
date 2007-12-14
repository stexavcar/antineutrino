#include "cctest/tests-inl.h"
#include "heap/values-inl.h"
#include "runtime/runtime.h"

using namespace neutrino;

void Test::tuples() {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  Tuple *tuple0 = cast<Tuple>(heap.new_tuple(0));
  CHECK_EQ(0, tuple0->length());
  Tuple *tuple1 = cast<Tuple>(heap.new_tuple(1));
  CHECK_EQ(1, tuple1->length());
  CHECK_IS(Void, tuple1->get(0));
  tuple1->set(0, tuple0);
  CHECK(tuple1->get(0) == tuple0);
  static int kCount = 100;
  Tuple *long_tuple = cast<Tuple>(heap.new_tuple(kCount));
  for (int i = 0; i < kCount; i++)
    CHECK_IS(Void, long_tuple->get(i));
  for (int i = 0; i < kCount; i++)
    long_tuple->set(i, Smi::from_int(i - 50));
  for (int i = 0; i < kCount; i++) {
    Value *val = long_tuple->get(i);
    CHECK_EQ(i - 50, cast<Smi>(val)->value());
  }
}

void Test::bounds_check() {
#ifdef DEBUG
  LocalRuntime runtime;
  Tuple *tuple = cast<Tuple>(runtime.heap().new_tuple(10));
  CHECK_ABORTS(OUT_OF_BOUNDS, tuple->get(10));
#endif  
}
