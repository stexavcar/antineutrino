#include "heap/values-inl.h"
#include "runtime/runtime.h"

using namespace neutrino;

void Test::equality() {
  Runtime runtime;
  runtime.initialize();
  Heap &heap = runtime.heap();
  
  CHECK_EQ(cast<Value>(heap.new_string("foo")), cast<Value>(heap.new_string("foo")));
}
