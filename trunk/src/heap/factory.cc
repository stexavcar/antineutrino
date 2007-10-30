#include "heap/factory.h"
#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "runtime/runtime.h"

namespace neutrino {

Factory::Factory(Runtime &runtime)
  : runtime_(runtime) { }

ref<String> Factory::new_string(string str) {
  Data *result = runtime().heap().new_string(str);
  return new_ref(cast<String>(result));
}

ref<Tuple> Factory::new_tuple(uint32_t size) {
  Data *result = runtime().heap().new_tuple(size);
  return new_ref(cast<Tuple>(result));
}

ref<Lambda> Factory::new_lambda(uint32_t argc, ref<Code> code,
    ref<Tuple> literals) {
  Data *result = runtime().heap().new_lambda(argc, *code, *literals);
  return new_ref(cast<Lambda>(result));
}

ref<Literal> Factory::new_literal(ref<Value> value) {
  Data *result = runtime().heap().new_literal(*value);
  return new_ref(cast<Literal>(result));
}

ref<Dictionary> Factory::new_dictionary() {
  Data *result = runtime().heap().new_dictionary();
  return new_ref(cast<Dictionary>(result));
}

ref<Code> Factory::new_code(uint32_t size) {
  Data *result = runtime().heap().new_code(size);
  return new_ref(cast<Code>(result));
}

}
