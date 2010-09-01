#include "cctest/nunit-inl.h"
#include "values/values-inl.pp.h"

using namespace neutrino;

TEST(signals) {
  Signal *val = AllocationFailed::make(100);
  CHECK(!is<Smi>(val));
  CHECK(!is<Object>(val));
  CHECK(is<Signal>(val));
  Signal *signal = cast<Signal>(val);
  @check signal->type() == Signal::sAllocationFailed;
  CHECK(is<AllocationFailed>(signal));
  CHECK(!is<FatalError>(signal));
  @check cast<AllocationFailed>(signal)->payload() == 100;
  Signal *ie = FatalError::make(FatalError::feUnexpected);
  CHECK(is<Signal>(ie));
  CHECK(is<FatalError>(ie));
  CHECK(!is<AllocationFailed>(ie));
  @check cast<FatalError>(ie)->payload() == FatalError::feUnexpected;
  Signal *no = Nothing::make();
  @check is<Nothing>(no);
}

TEST(forward_pointers) {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  Code *code = heap.new_code(4).value();
  ForwardPointer *ptr = ForwardPointer::make(code);
  CHECK(is<ForwardPointer>(ptr));
  Object *target = ptr->target();
  CHECK(code == target);
}

TEST(type_mismatch) {
  Signal *val = TypeMismatch::make(tString, tTuple);
  @check is<TypeMismatch>(val);
  @check cast<TypeMismatch>(val)->expected() == tString;
  @check cast<TypeMismatch>(val)->found() == tTuple;
}
