#include "cctest/tests-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::equality() {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();
  
  CHECK_EQ(cast<Value>(heap.new_string("foo")), cast<Value>(heap.new_string("foo")));
}
