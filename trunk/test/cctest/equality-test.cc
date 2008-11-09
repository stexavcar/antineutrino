#include "cctest/nunit-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.pp.h"

using namespace neutrino;

TEST(equality) {
  LocalRuntime runtime;
  Heap &heap = runtime.heap();

  @check cast<Value>(heap.new_string("foo").value()) == cast<Value>(heap.new_string("foo").value());
}
