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

ref<Symbol> Factory::new_symbol(ref<Value> name) {
  ALLOCATE_CHECKED(Symbol, new_symbol(*name));
}

ref<Lambda> Factory::new_lambda(uint32_t argc, ref<Value> code,
    ref<Value> literals, ref<Value> tree, ref<Context> context) {
  ALLOCATE_CHECKED(Lambda, new_lambda(argc, *code, *literals, *tree, *context));
}

ref<QuoteTemplate> Factory::new_quote_template(ref<SyntaxTree> body, ref<Tuple> unquotes) {
  ALLOCATE_CHECKED(QuoteTemplate, new_quote_template(*body, *unquotes));
}

ref<LambdaExpression> Factory::new_lambda_expression(ref<Tuple> params,
    ref<SyntaxTree> body) {
  ALLOCATE_CHECKED(LambdaExpression, new_lambda_expression(*params, *body));
}

ref<ReturnExpression> Factory::new_return_expression(ref<SyntaxTree> value) {
  ALLOCATE_CHECKED(ReturnExpression, new_return_expression(*value));
}

ref<LiteralExpression> Factory::new_literal_expression(ref<Value> value) {
  ALLOCATE_CHECKED(LiteralExpression, new_literal_expression(*value));
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

ref<Layout> Factory::allocate_empty_layout(InstanceType instance_type) {
  ALLOCATE_CHECKED(Layout, allocate_empty_layout(instance_type));
}

ref<Layout> Factory::new_layout(InstanceType instance_type,
    uint32_t instance_field_count, ref<Tuple> methods, ref<Value> super,
    ref<Value> name) {
  ALLOCATE_CHECKED(Layout, new_layout(instance_type, instance_field_count, *methods, *super, *name));
}

ref<Instance> Factory::new_instance(ref<Layout> layout) {
  ALLOCATE_CHECKED(Instance, new_instance(*layout));
}

}
