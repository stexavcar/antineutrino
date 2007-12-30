#include "runtime/runtime.h"
#include "heap/values-inl.h"

using namespace neutrino;

void Test::runtime_creation() {
  Runtime runtime;
  CHECK(runtime.initialize());
  CHECK_IS(Layout, runtime.roots().layout_layout());
  CHECK_IS(Layout, runtime.roots().string_layout());
}
