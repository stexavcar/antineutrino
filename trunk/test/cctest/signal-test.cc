#include "cctest/tests-inl.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::signals() {
  Signal *val = AllocationFailed::make(100);
  CHECK(!is<Smi>(val));
  CHECK(!is<Object>(val));
  CHECK(is<Signal>(val));
  Signal *signal = cast<Signal>(val);
  CHECK_EQ(Signal::sAllocationFailed, signal->type());
  CHECK(is<AllocationFailed>(signal));
  CHECK(!is<InternalError>(signal));
  CHECK_EQ(100, cast<AllocationFailed>(signal)->payload());
  Signal *ie = InternalError::make(25);
  CHECK(is<Signal>(ie));
  CHECK(is<InternalError>(ie));
  CHECK(!is<AllocationFailed>(ie));
  CHECK_EQ(25, cast<InternalError>(ie)->payload());
  Signal *no = Nothing::make();
  CHECK_IS(Nothing, no);
}

void Test::forward_pointers() {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  Code *code = cast<Code>(heap.new_code(4));
  ForwardPointer *ptr = ForwardPointer::make(code);
  CHECK(is<ForwardPointer>(ptr));
  Object *target = ptr->target();
  CHECK(code == target);
}
