#include "runtime/runtime.h"
#include "heap/values-inl.h"

using namespace neutrino;

static void test_runtime_creation() {
  Runtime runtime;
  CHECK(runtime.initialize());
  CHECK_IS(Type, runtime.roots().type_type());
  CHECK_IS(Type, runtime.roots().string_type());
}
