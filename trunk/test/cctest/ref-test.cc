#include "heap/ref-inl.h"
#include "heap/heap.h"
#include "runtime/runtime.h"
#include "cctest/tests-inl.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::handles() {
  RefStack refs;
  ref_scope scope(refs);
  Runtime runtime;
  runtime.initialize();
  Heap &heap = runtime.heap();
  Tuple *tuple = cast<Tuple>(heap.new_tuple(10));
  ref<Tuple> tuple_ref = refs.new_ref(tuple);
  CHECK_EQ(10, tuple_ref->length());
  ref<Value> zero = tuple_ref.get(refs, 0);
  CHECK(is<Null>(zero));
  tuple_ref->set(1, *tuple_ref);
  ref<Value> one = tuple_ref.get(refs, 1);
  ref<Tuple> one_tuple = cast<Tuple>(one);
  ref<Value> one_tuple_value = one_tuple;
}

void Test::unscoped() {
#ifdef DEBUG
  RefStack refs;
  Value *value = Smi::from_int(0);
  CHECK_ABORTS(cnNoRefScope, refs.new_ref(value));
#endif // DEBUG
}

static void test_deep(RefStack &refs, uword n) {
  if (n == 0) return;
  ref_scope scope(refs);
  static const int kCount = 300;
  ref<Smi> rs[kCount];
  for (int i = 0; i < kCount; i++)
    rs[i] = refs.new_ref(Smi::from_int(n + (i << 16)));
  test_deep(refs, n - 1);
  for (int i = 0; i < kCount; i++) {
    CHECK_EQ(rs[i]->value(), n + (i << 16));
  }
}

void Test::deep() {
  RefStack refs;
  test_deep(refs, 500);
}

static void count_refs(RefStack &refs, uword expected) {
  ref_iterator iter(refs);
  uword count = 0;
  while (iter.has_next()) {
    CHECK(count < expected);
    Value *val = iter.next();
    CHECK_IS(Smi, val);
    CHECK_EQ(count, cast<Smi>(val)->value());
    count++;
  }
  CHECK_EQ(count, expected);
}

void Test::ref_iteration() {
  RefStack refs;
  ref_scope scope(refs);
  const uword kRefCount = 1024;
  for (uword i = 0; i < kRefCount; i++) {
    count_refs(refs, i);
    ref<Smi> next = refs.new_ref(Smi::from_int(i));
  }
  count_refs(refs, kRefCount);
}
