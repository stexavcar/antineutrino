#include "runtime/runtime.h"
#include "heap/values-inl.h"

using namespace neutrino;

void Test::runtime_creation() {
  Runtime runtime;
  CHECK(runtime.initialize());
  CHECK_IS(Class, runtime.roots().class_class());
  CHECK_IS(Class, runtime.roots().string_class());
}
