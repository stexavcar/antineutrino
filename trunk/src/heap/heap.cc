#include "compiler/ast-inl.h"
#include "heap/heap-inl.h"
#include "heap/pointer.h"
#include "heap/roots.h"
#include "heap/values-inl.h"
#include "utils/string-inl.h"
#include "utils/types-inl.h"

using namespace neutrino;

Heap::Heap(Roots &roots)
    : roots_(roots) { }

Data *Heap::allocate_object(uint32_t size, Class *type) {
  address addr = space().allocate_raw(size);
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

Data *Heap::new_lambda(uint32_t argc, Code *code, Tuple *literals) {
  Data *val = allocate_object(Lambda::kSize, roots().lambda_class());
  if (is<AllocationFailed>(val)) return val;
  Lambda *result = cast<Lambda>(val);
  result->set_argc(argc);
  result->set_code(code);
  result->set_literals(literals);
  return result;
}

Data *Heap::new_lambda(uint32_t argc) {
  Data *val = allocate_object(Lambda::kSize, roots().lambda_class());
  if (is<AllocationFailed>(val)) return val;
  Lambda *result = cast<Lambda>(val);
  result->set_argc(argc);
  return result;
}

Data *Heap::new_literal_expression() {
  Data *val = allocate_object(LiteralExpression::kSize, roots().literal_expression_class());
  if (is<AllocationFailed>(val)) return val;
  LiteralExpression *result = cast<LiteralExpression>(val);
  return result;
}


Data *Heap::new_invoke_expression() {
  Data *val = allocate_object(InvokeExpression::kSize, roots().invoke_expression_class());
  if (is<AllocationFailed>(val)) return val;
  InvokeExpression *result = cast<InvokeExpression>(val);
  return result;
}


Data *Heap::new_class_expression() {
  Data *val = allocate_object(ClassExpression::kSize, roots().class_expression_class());
  if (is<AllocationFailed>(val)) return val;
  ClassExpression *result = cast<ClassExpression>(val);
  return result;
}


Data *Heap::new_return_expression() {
  Data *val = allocate_object(ReturnExpression::kSize, roots().return_expression_class());
  if (is<AllocationFailed>(val)) return val;
  ReturnExpression *result = cast<ReturnExpression>(val);
  return result;
}


Data *Heap::new_method_expression() {
  Data *val = allocate_object(MethodExpression::kSize, roots().method_expression_class());
  if (is<AllocationFailed>(val)) return val;
  MethodExpression *result = cast<MethodExpression>(val);
  return result;
}


Data *Heap::new_sequence_expression() {
  Data *val = allocate_object(SequenceExpression::kSize, roots().sequence_expression_class());
  if (is<AllocationFailed>(val)) return val;
  SequenceExpression *result = cast<SequenceExpression>(val);
  return result;
}


Data *Heap::new_tuple_expression() {
  Data *val = allocate_object(TupleExpression::kSize, roots().tuple_expression_class());
  if (is<AllocationFailed>(val)) return val;
  TupleExpression *result = cast<TupleExpression>(val);
  return result;
}

Data *Heap::new_global_expression() {
  Data *val = allocate_object(GlobalExpression::kSize, roots().global_expression_class());
  if (is<AllocationFailed>(val)) return val;
  GlobalExpression *result = cast<GlobalExpression>(val);
  return result;
}

Data *Heap::new_empty_class(InstanceType instance_type) {
  Data *val = allocate_class(instance_type);
  if (is<AllocationFailed>(val)) return val;
  ASSERT_IS(Class, val);
  Class *result = cast<Class>(val);
  result->set_super(Smi::from_int(0));
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

Data *Heap::new_method() {
  Data *val = allocate_object(Method::kSize, roots().method_class());
  if (is<AllocationFailed>(val)) return val;
  Method *result = cast<Method>(val);
  return result;
}

Data *Heap::new_string(string value) {
  int size = String::size_for(value.length());
  Data *val = allocate_object(size, roots().string_class());
  if (is<AllocationFailed>(val)) return val;
  String *result = cast<String>(val);
  result->set_length(value.length());
  for (uint32_t i = 0; i < value.length(); i++)
    result->set(i, value[i]);
  return result;
}

Data *Heap::new_string(uint32_t length) {
  int size = String::size_for(length);
  Data *val = allocate_object(size, roots().string_class());
  if (is<AllocationFailed>(val)) return val;
  String *result = cast<String>(val);
  result->set_length(length);
  return result;
}

Data *Heap::new_code(uint32_t size) {
  return new_abstract_buffer(sizeof(uint16_t) * size, roots().code_class());
}

Data *Heap::new_abstract_buffer(uint32_t byte_count, Class *type) {
  int size = AbstractBuffer::size_for(byte_count);
  Data *val = allocate_object(size, type);
  if (is<AllocationFailed>(val)) return val;
  AbstractBuffer *result = cast<AbstractBuffer>(val);
  result->set_size<uint8_t>(byte_count);
  return result;
}

Data *Heap::new_tuple(uint32_t length) {
  int size = Tuple::size_for(length);
  Data *val = allocate_object(size, roots().tuple_class());
  if (is<AllocationFailed>(val)) return val;
  Tuple *result = cast<Tuple>(val);
  result->set_length(length);
  for (uint32_t i = 0; i < length; i++)
    result->set(i, roots().vhoid());
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
