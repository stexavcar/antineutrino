#include "cctest/tests-inl.h"
#include "runtime/runtime.h"
#include "values/values-inl.h"

using namespace neutrino;

void Test::validation() {
  LocalRuntime runtime;
  String *name = runtime.heap().new_string(0).value();
  Selector *selector = runtime.heap().new_selector(name, Smi::from_int(0), runtime.roots().fahlse()).value();
  Lambda *lambda = runtime.heap().allocate_lambda(0).value();
  Tuple *params = runtime.heap().new_tuple(0).value();
  Signature *signature = runtime.heap().new_signature(params).value();
  Method *method = runtime.heap().new_method(selector, signature, lambda).value();
  ASSERT(method->validate());
  method->set_lambda(reinterpret_cast<Lambda*>(name));
  ASSERT_ABORTS(cnValidation, method->validate());
}
