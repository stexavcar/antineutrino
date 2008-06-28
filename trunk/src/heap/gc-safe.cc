#include "heap/gc-safe.h"
#include "heap/ref-inl.h"
#include "values/values-inl.h"
#include "runtime/runtime.h"

namespace neutrino {


// ---------------------
// --- G C   S a f e ---
// ---------------------


/**
 * Performs the specified operation in a gc-protect manner without
 * returning a value.
 */
#define DO_CHECKED(operation) do {                                   \
  Data *result = operation;                                          \
  if (is<AllocationFailed>(result)) {                                \
    runtime().heap().memory().collect_garbage(runtime());            \
    result = operation;                                              \
    if (is<AllocationFailed>(result)) {                              \
      UNREACHABLE();                                                 \
    }                                                                \
  }                                                                  \
} while (false)


/**
 * Performs the specified operation in a gc-protect manner, wraps the
 * result in a handle and returns it.
 */
#define RETURN_CHECKED(Type, operation) do {                         \
  Data *result = operation;                                          \
  if (is<AllocationFailed>(result)) {                                \
    runtime().heap().memory().collect_garbage(runtime());            \
    result = operation;                                              \
    if (is<AllocationFailed>(result)) {                              \
      return InternalError::make(InternalError::ieOutOfMemory);      \
    }                                                                \
  }                                                                  \
  return cast<Type>(result);                                         \
} while (false)


GcSafe::GcSafe(Runtime &runtime)
    : runtime_(runtime) { }


void GcSafe::set(ref<HashMap> dict, ref<Value> key, ref<Value> value) {
  DO_CHECKED(dict->set(runtime().heap(), *key, *value));
}


/**
 * Shorthand for performing a gc-protect allocation.
 */
#define ALLOCATE_CHECKED(Type, allocator)                            \
  RETURN_CHECKED(Type, runtime().heap().allocator)


Factory::Factory(Runtime &runtime)
  : runtime_(runtime) { }


Data *Factory::new_string(string str) {
  ALLOCATE_CHECKED(String, new_string(str));
}


Data *Factory::new_string(uword length) {
  ALLOCATE_CHECKED(String, new_string(length));
}


Data *Factory::new_tuple(uword size) {
  ALLOCATE_CHECKED(Tuple, new_tuple(size));
}


Data *Factory::new_symbol(ref<Value> name) {
  ALLOCATE_CHECKED(Symbol, new_symbol(*name));
}


Data *Factory::new_lambda(uword argc, uword max_stack_height,
    ref<Value> code, ref<Value> literals, ref<Value> tree,
    ref<Context> context) {
  ALLOCATE_CHECKED(Lambda, new_lambda(argc, max_stack_height, *code, *literals, *tree, *context));
}


Data *Factory::new_task(Architecture &arch) {
  ALLOCATE_CHECKED(Task, new_task(arch));
}


Data *Factory::new_parameters(ref<Smi> position_count,
    ref<Tuple> params) {
  ALLOCATE_CHECKED(Parameters, new_parameters(*position_count, *params));
}


Data *Factory::new_lambda_expression(ref<Parameters> params,
    ref<SyntaxTree> body, bool is_local) {
  ALLOCATE_CHECKED(LambdaExpression, new_lambda_expression(*params, *body, is_local));
}


Data *Factory::new_return_expression(ref<SyntaxTree> value) {
  ALLOCATE_CHECKED(ReturnExpression, new_return_expression(*value));
}


Data *Factory::new_literal_expression(ref<Value> value) {
  ALLOCATE_CHECKED(LiteralExpression, new_literal_expression(*value));
}


Data *Factory::new_hash_map() {
  ALLOCATE_CHECKED(HashMap, new_hash_map());
}


Data *Factory::new_code(uword size) {
  ALLOCATE_CHECKED(Code, new_code(size));
}


Data *Factory::new_signature(ref<Tuple> parameters) {
  ALLOCATE_CHECKED(Signature, new_signature(*parameters));
}


Data *Factory::new_method(ref<Selector> selector, ref<Signature> signature,
    ref<Lambda> lambda) {
  ALLOCATE_CHECKED(Method, new_method(*selector, *signature, *lambda));
}


Data *Factory::allocate_empty_layout(InstanceType instance_type) {
  ALLOCATE_CHECKED(Layout, allocate_empty_layout(instance_type));
}


Data *Factory::new_layout(InstanceType instance_type,
    uword instance_field_count, ref<Immediate> protocol, ref<Tuple> methods) {
  ALLOCATE_CHECKED(Layout, new_layout(instance_type, instance_field_count, *protocol, *methods));
}


Data *Factory::new_protocol(ref<Tuple> methods, ref<Value> super,
    ref<Immediate> name) {
  ALLOCATE_CHECKED(Protocol, new_protocol(*methods, *super, *name));
}


Data *Factory::new_instance(ref<Layout> layout) {
  ALLOCATE_CHECKED(Instance, new_instance(*layout));
}


Data *Factory::new_selector(ref<Immediate> name, Smi *argc,
    ref<Bool> is_accessor) {
  ALLOCATE_CHECKED(Selector, new_selector(*name, argc, *is_accessor));
}


} // neutrino
