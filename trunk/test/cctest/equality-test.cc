#include "cctest/nunit-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.h"

using namespace neutrino;

TEST(equality) {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();

  CHECK_EQ(cast<Value>(heap.new_string("foo").value()), cast<Value>(heap.new_string("foo").value()));
}
