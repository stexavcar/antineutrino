#include "heap/factory.h"
#include "heap/ref-inl.h"
#include "heap/values-inl.h"
#include "runtime/runtime.h"

namespace neutrino {

Factory::Factory(Runtime &runtime)
  : runtime_(runtime) { }

#define ALLOCATE_CHECKED(Type, operation) do {                       \
  Data *result = runtime().heap().operation;                         \
  if (is<AllocationFailed>(result)) {                                \
    runtime().heap().memory().collect_garbage();                     \
    result = runtime().heap().operation;                             \
    if (is<AllocationFailed>(result)) {                              \
      UNREACHABLE();                                                 \
    }                                                                \
  }                                                                  \
  return new_ref(cast<Type>(result));                                \
} while (false)

ref<String> Factory::new_string(string str) {
  ALLOCATE_CHECKED(String, new_string(str));
}

ref<String> Factory::new_string(uint32_t length) {
  ALLOCATE_CHECKED(String, new_string(length));
}

ref<Tuple> Factory::new_tuple(uint32_t size) {
  ALLOCATE_CHECKED(Tuple, new_tuple(size));
}

ref<Lambda> Factory::new_lambda(uint32_t argc, ref<Value> code,
    ref<Value> literals, ref<LambdaExpression> tree) {
  ALLOCATE_CHECKED(Lambda, new_lambda(argc, *code, *literals, *tree));
}

ref<Dictionary> Factory::new_dictionary() {
  ALLOCATE_CHECKED(Dictionary, new_dictionary());
}

ref<Code> Factory::new_code(uint32_t size) {
  ALLOCATE_CHECKED(Code, new_code(size));
}

ref<Method> Factory::new_method(ref<String> name, ref<Lambda> lambda) {
  ALLOCATE_CHECKED(Method, new_method(*name, *lambda));
}

ref<Class> Factory::new_empty_class(InstanceType instance_type) {
  ALLOCATE_CHECKED(Class, new_empty_class(instance_type));
}

ref<Instance> Factory::new_instance(ref<Class> chlass) {
  ALLOCATE_CHECKED(Instance, new_instance(*chlass));
}

}
