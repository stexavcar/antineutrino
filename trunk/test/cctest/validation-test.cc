#include "cctest/tests-inl.h"
#include "heap/values-inl.h"
#include "runtime/runtime.h"

using namespace neutrino;

void Test::validation() {
  LocalRuntime runtime;
  String *name = cast<String>(runtime.heap().new_string(0));
  Lambda *lambda = cast<Lambda>(runtime.heap().allocate_lambda(0));
  Method *method = cast<Method>(runtime.heap().new_method(name, lambda));
  ASSERT(method->validate());
  method->set_lambda(reinterpret_cast<Lambda*>(name));
  ASSERT_ABORTS(VALIDATION, method->validate());
}
