#include "compiler/ast-inl.h"
#include "heap/heap-inl.h"
#include "heap/memory-inl.h"
#include "heap/pointer.h"
#include "heap/roots.h"
#include "heap/values-inl.h"
#include "utils/string-inl.h"
#include "utils/types-inl.h"

using namespace neutrino;

Heap::Heap(Roots &roots)
    : roots_(roots)
    , memory_(*this) { }

Data *Heap::allocate_object(uint32_t size, Class *type) {
  address addr = memory().allocate(size);
  if (!addr) return AllocationFailed::make(size);
  Object *result = ValuePointer::tag_as_object(addr);
  result->set_chlass(type);
#ifdef DEBUG
  uint32_t *fields = reinterpret_cast<uint32_t*>(addr);
  for (uint32_t i = 1; i < size / kPointerSize; i++)
    fields[i] = ValuePointer::kUninitialized;
#endif
  return result;
}

Data *Heap::allocate_class(InstanceType instance_type) {
  Data *val = allocate_object(Class::kSize, roots().class_class());
  if (is<AllocationFailed>(val)) return val;
  Class *result = reinterpret_cast<Class*>(cast<Object>(val));
  result->set_chlass(roots().class_class());
  result->set_instance_type(instance_type);
  result->set_super(Smi::from_int(0));
  return result;
}

Data *Heap::new_lambda(uint32_t argc, Value *code, Value *literals, LambdaExpression *tree) {
  Data *val = allocate_object(Lambda::kSize, roots().lambda_class());
  if (is<AllocationFailed>(val)) return val;
  Lambda *result = cast<Lambda>(val);
  result->set_argc(argc);
  result->set_code(code);
  result->set_literals(literals);
  result->set_tree(tree);
  result->set_outers(roots().empty_tuple());
  return result;
}

Data *Heap::new_quote_template(SyntaxTree *value, Tuple *unquotes) {
  Data *val = allocate_object(QuoteTemplate::kSize, roots().quote_template_class());
  if (is<AllocationFailed>(val)) return val;
  QuoteTemplate *result = cast<QuoteTemplate>(val);
  result->set_value(value);
  result->set_unquotes(unquotes);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::new_lambda_expression(Tuple *params, SyntaxTree *body) {
  Data *val = allocate_object(LambdaExpression::kSize, roots().lambda_expression_class());
  if (is<AllocationFailed>(val)) return val;
  LambdaExpression *result = cast<LambdaExpression>(val);
  result->set_params(params);
  result->set_body(body);
  return result;
}

Data *Heap::new_return_expression(SyntaxTree *value) {
  Data *val = allocate_object(ReturnExpression::kSize, roots().return_expression_class());
  if (is<AllocationFailed>(val)) return val;
  ReturnExpression *result = cast<ReturnExpression>(val);
  result->set_value(value);
  IF_PARANOID(result->validate());
  return result;
}

Data *Heap::allocate_lambda(uint32_t argc) {
  Data *val = allocate_object(Lambda::kSize, roots().lambda_class());
  if (is<AllocationFailed>(val)) return val;
  Lambda *result = cast<Lambda>(val);
  result->set_argc(argc);
  return result;
}

Data *Heap::allocate_builtin_call() {
  return allocate_object(BuiltinCall::kSize, roots().builtin_call_class());
}

Data *Heap::allocate_unquote_expression() {
  return allocate_object(UnquoteExpression::kSize, roots().unquote_expression_class());
}

#define MAKE_ALLOCATOR(n, NAME, Name, name)                          \
Data *Heap::allocate_##name() {                                      \
  return allocate_object(Name::kSize, roots().name##_class());       \
}
FOR_EACH_GENERATABLE_TYPE(MAKE_ALLOCATOR)
#undef MAKE_ALLOCATOR

Data *Heap::new_empty_class(InstanceType instance_type) {
  Data *val = allocate_class(instance_type);
  if (is<AllocationFailed>(val)) return val;
  ASSERT_IS(Class, val);
  Class *result = cast<Class>(val);
  result->set_super(Smi::from_int(0));
  result->set_methods(roots().empty_tuple());
  ASSERT(result->is_empty());
  return result;
}

Data *Heap::new_method(String *name, Lambda *lambda) {
  Data *val = allocate_object(Method::kSize, roots().method_class());
  if (is<AllocationFailed>(val)) return val;
  Method *result = cast<Method>(val);
  result->set_name(name);
  result->set_lambda(lambda);
  return result;
}

Data *Heap::new_string(string value) {
  uint32_t size = String::size_for(value.length());
  Data *val = allocate_object(size, roots().string_class());
  if (is<AllocationFailed>(val)) return val;
  String *result = cast<String>(val);
  result->set_length(value.length());
  for (uint32_t i = 0; i < value.length(); i++)
    result->set(i, value[i]);
  ASSERT_EQ(size, result->size_in_memory());
  return result;
}

Data *Heap::new_string(uint32_t length) {
  uint32_t size = String::size_for(length);
  Data *val = allocate_object(size, roots().string_class());
  if (is<AllocationFailed>(val)) return val;
  String *result = cast<String>(val);
  result->set_length(length);
  ASSERT_EQ(size, result->size_in_memory());
  return result;
}

Data *Heap::new_code(uint32_t size) {
  return new_abstract_buffer(sizeof(uint16_t) * size, roots().code_class());
}

Data *Heap::new_abstract_buffer(uint32_t byte_count, Class *type) {
  uint32_t size = AbstractBuffer::size_for(byte_count);
  Data *val = allocate_object(size, type);
  if (is<AllocationFailed>(val)) return val;
  AbstractBuffer *result = cast<AbstractBuffer>(val);
  result->set_size<uint8_t>(byte_count);
  ASSERT_EQ(size, result->size_in_memory());
  return result;
}

Data *Heap::new_tuple(uint32_t length) {
  uint32_t size = Tuple::size_for(length);
  Data *val = allocate_object(size, roots().tuple_class());
  if (is<AllocationFailed>(val)) return val;
  Tuple *result = cast<Tuple>(val);
  result->set_length(length);
  for (uint32_t i = 0; i < length; i++)
    result->set(i, roots().vhoid());
  ASSERT_EQ(size, result->size_in_memory());
  return result;
}

Data *Heap::new_singleton(Class *type) {
  return allocate_object(Singleton::kSize, type);
}

Data *Heap::new_dictionary() {
  Data *table_val = new_tuple(0);
  if (is<AllocationFailed>(table_val)) return table_val;
  return new_dictionary(cast<Tuple>(table_val));
}

Data *Heap::new_dictionary(Tuple *table) {
  Data *val = allocate_object(Dictionary::kSize, roots().dictionary_class());
  if (is<AllocationFailed>(val)) return val;
  Dictionary *result = cast<Dictionary>(val);
  result->set_table(table);
  return result;
}

Data *Heap::new_instance(Class *chlass) {
  ASSERT_EQ(INSTANCE_TYPE, chlass->instance_type());
  uint32_t size = Instance::kHeaderSize;
  Data *val = allocate_object(size, chlass);
  if (is<AllocationFailed>(val)) return val;
  Instance *result = cast<Instance>(val);
  return result;
}
