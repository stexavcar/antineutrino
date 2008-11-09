#include "cctest/nunit-inl.h"
#include "heap/ref-inl.h"
#include "heap/heap.h"
#include "runtime/runtime.h"
#include "values/values-inl.pp.h"

using namespace neutrino;

TEST(handles) {
  RefManager refs;
  ref_block<> protect(refs);
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  Tuple *tuple = heap.new_tuple(10).value();
  ref<Tuple> tuple_ref = protect(tuple);
  @check tuple_ref->length() == 10;
  ref<Value> zero = protect(tuple_ref->get(0));
  CHECK(is<Null>(zero));
  tuple_ref->set(1, *tuple_ref);
  ref<Value> one = protect(tuple_ref->get(1));
  ref<Tuple> one_tuple = cast<Tuple>(one);
  ref<Value> one_tuple_value = one_tuple;
}

TEST(too_many_refs) {
#ifdef DEBUG
  RefManager refs;
  ref_block<2> protect(refs);
  protect(Smi::from_int(0));
  protect(Smi::from_int(1));
  CHECK_ABORTS(cnRefOverflow, protect(Smi::from_int(2)));
#endif
}

static void test_deep(RefManager &refs, uword n) {
  static const int kCount = 300;
  if (n == 0) return;
  ref_block<kCount> protect(refs);
  ref<Smi> rs[kCount];
  for (int i = 0; i < kCount; i++)
    rs[i] = protect(Smi::from_int(n + (i << 16)));
  test_deep(refs, n - 1);
  for (int i = 0; i < kCount; i++) {
    @check n + (i << 16) == rs[i]->value();
  }
}

TEST(deep) {
  RefManager refs;
  test_deep(refs, 500);
}

static void count_refs(RefManager &refs, uword expected) {
  ref_iterator iter(refs);
  uword count = 0;
  while (iter.has_next()) {
    CHECK(count < expected);
    Value *val = iter.next();
    @check is<Smi>(val);
    @check cast<Smi>(val)->value() == count;
    count++;
  }
  @check expected == count;
}

TEST(ref_iteration) {
  static const uword kRefCount = 1024;
  RefManager refs;
  ref_block<kRefCount> protect(refs);
  for (uword i = 0; i < kRefCount; i++) {
    count_refs(refs, i);
    ref<Smi> next = protect(Smi::from_int(i));
  }
  count_refs(refs, kRefCount);
}
