#include "cctest/tests-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::validation() {
  LocalRuntime runtime;
  String *name = cast<String>(runtime.heap().new_string(0));
  Selector *selector = cast<Selector>(runtime.heap().new_selector(name, Smi::from_int(0), runtime.roots().fahlse()));
  Lambda *lambda = cast<Lambda>(runtime.heap().allocate_lambda(0));
  Tuple *params = cast<Tuple>(runtime.heap().new_tuple(0));
  Signature *signature = cast<Signature>(runtime.heap().new_signature(params));
  Method *method = cast<Method>(runtime.heap().new_method(selector, signature, lambda));
  ASSERT(method->validate());
  method->set_lambda(reinterpret_cast<Lambda*>(name));
  ASSERT_ABORTS(cnValidation, method->validate());
}
