#include "cctest/tests-inl.h"
#include "heap/values-inl.h"
#include "runtime/runtime.h"

using namespace neutrino;

static void test_bounds_check() {
#ifdef DEBUG
  LocalRuntime runtime;
  String *name = cast<String>(runtime.heap().new_string(0));
  Lambda *lambda = cast<Lambda>(runtime.heap().allocate_lambda(0));
  Method *method = cast<Method>(runtime.heap().new_method(name, lambda));
  method->validate();
  method->set_lambda(reinterpret_cast<Lambda*>(name));
  CHECK_ABORTS(VALIDATION, method->validate());
#endif
}
