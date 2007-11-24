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

ref<Lambda> Factory::new_lambda(uint32_t argc, ref<Value> code,
    ref<Value> literals, ref<LambdaExpression> tree) {
  Data *result = runtime().heap().new_lambda(argc, *code, *literals, *tree);
  return new_ref(cast<Lambda>(result));
}

ref<Dictionary> Factory::new_dictionary() {
  Data *result = runtime().heap().new_dictionary();
  return new_ref(cast<Dictionary>(result));
}

ref<Code> Factory::new_code(uint32_t size) {
  Data *result = runtime().heap().new_code(size);
  return new_ref(cast<Code>(result));
}

ref<Method> Factory::new_method(ref<String> name, ref<Lambda> lambda) {
  Data *result = runtime().heap().new_method(*name, *lambda);
  return new_ref(cast<Method>(result));
}

ref<Class> Factory::new_empty_class(InstanceType instance_type) {
  Data *result = runtime().heap().new_empty_class(instance_type);
  return new_ref(cast<Class>(result));
}

ref<Instance> Factory::new_instance(ref<Class> chlass) {
  Data *result = runtime().heap().new_instance(*chlass);
  return new_ref(cast<Instance>(result));
}

}
