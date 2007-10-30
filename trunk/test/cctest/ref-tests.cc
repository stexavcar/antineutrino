#include "heap/ref-inl.h"
#include "heap/heap.h"
#include "heap/values-inl.h"
#include "runtime/runtime.h"
#include "cctest/tests-inl.h"

using namespace neutrino;

static void test_handles() {
  RefScope scope;
  Runtime runtime;
  runtime.initialize();
  Heap &heap = runtime.heap();
  Tuple *tuple = cast<Tuple>(heap.new_tuple(10));
  ref<Tuple> tuple_ref = new_ref(tuple);
  CHECK_EQ(10, tuple_ref->length());
  ref<Value> zero = tuple_ref.get(0);
  CHECK(is<Void>(zero));
  tuple_ref->at(1) = *tuple_ref;
  ref<Value> one = tuple_ref.get(1);
  ref<Tuple> one_tuple = cast<Tuple>(one);
  ref<Value> one_tuple_value = one_tuple;
}

static void test_unscoped() {
#ifdef DEBUG
  Value *value = Smi::from_int(0);
  CHECK_ABORTS(NO_REF_SCOPE, new_ref(value));
#endif // DEBUG
}

static void test_deep(uint32_t n) {
  if (n == 0) return;
  RefScope scope;
  static const int kCount = 300;
  ref<Smi> refs[kCount];
  for (int i = 0; i < kCount; i++)
    refs[i] = new_ref(Smi::from_int(n + i << 16));
  test_deep(n - 1);
  for (int i = 0; i < kCount; i++) {
    ASSERT_EQ(refs[i]->value(), n + i << 16);
  }
}

static void test_deep() {
  test_deep(1234);
}
