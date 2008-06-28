#include "cctest/tests-inl.h"
#include "heap/ref-inl.h"
#include "heap/heap.h"
#include "runtime/runtime.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::handles() {
  RefStack refs;
  stack_ref_block<> safe(refs);
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  Tuple *tuple = cast<Tuple>(heap.new_tuple(10));
  ref<Tuple> tuple_ref = safe(tuple);
  CHECK_EQ(10, tuple_ref->length());
  ref<Value> zero = safe(tuple_ref.get(0));
  CHECK(is<Null>(zero));
  tuple_ref->set(1, *tuple_ref);
  ref<Value> one = safe(tuple_ref.get(1));
  ref<Tuple> one_tuple = cast<Tuple>(one);
  ref<Value> one_tuple_value = one_tuple;
}

void Test::too_many_refs() {
#ifdef DEBUG
  RefStack refs;
  stack_ref_block<2> safe(refs);
  safe(Smi::from_int(0));
  safe(Smi::from_int(1));
  CHECK_ABORTS(cnRefOverflow, safe(Smi::from_int(2)));
#endif
}

static void test_deep(RefStack &refs, uword n) {
  static const int kCount = 300;
  if (n == 0) return;
  stack_ref_block<kCount> safe(refs);
  ref<Smi> rs[kCount];
  for (int i = 0; i < kCount; i++)
    rs[i] = safe(Smi::from_int(n + (i << 16)));
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
  static const uword kRefCount = 1024;
  RefStack refs;
  stack_ref_block<kRefCount> safe(refs);
  for (uword i = 0; i < kRefCount; i++) {
    count_refs(refs, i);
    ref<Smi> next = safe(Smi::from_int(i));
  }
  count_refs(refs, kRefCount);
}
