#include "heap/heap-inl.h"
#include "heap/pointer.h"
#include "heap/roots.h"
#include "heap/values-inl.h"
#include "io/ast-inl.h"
#include "utils/string-inl.h"
#include "utils/types-inl.h"

using namespace neutrino;

Heap::Heap(RootContainer &roots)
    : roots_(roots) { }

Data *Heap::allocate_object(uint32_t size, Type *type) {
  address addr = space().allocate_raw(size);
  Object *result = ValuePointer::tag_as_object(addr);
  result->set_type(type);
  return result;
}

Data *Heap::allocate_type(TypeTag instance_type) {
  Data *val = allocate_object(Type::kSize, roots().type_type());
  if (is<AllocationFailed>(val)) return val;
  Type *result = reinterpret_cast<Type*>(cast<Object>(val));
  result->set_type(roots().type_type());
  result->instance_type() = instance_type;
  return result;  
}

Data *Heap::new_lambda(uint32_t argc, Code *code, Tuple *literals) {
  Data *val = allocate_object(Lambda::kSize, roots().lambda_type());
  if (is<AllocationFailed>(val)) return val;
  Lambda *result = cast<Lambda>(val);
  result->argc() = argc;
  result->set_code(code);
  result->set_literals(literals);
  return result;
}

Data *Heap::new_literal(Value *value) {
  Data *val = allocate_object(Literal::kSize, roots().literal_type());
  if (is<AllocationFailed>(val)) return val;
  Literal *result = cast<Literal>(val);
  result->value() = value;
  return result;
}


Data *Heap::new_type(TypeTag instance_type) {
  Data *result = allocate_type(instance_type);
  if (is<AllocationFailed>(result)) return result;
  ASSERT_IS(Type, result);
  return result;
}

Data *Heap::new_string(string value) {
  int size = String::size_for(value.length());
  Data *val = allocate_object(size, roots().string_type());
  if (is<AllocationFailed>(val)) return val;
  String *result = cast<String>(val);
  result->length() = value.length();
  for (uint32_t i = 0; i < value.length(); i++)
    result->at(i) = value[i];
  return result;
}

Data *Heap::new_code(uint32_t size) {
  return new_abstract_buffer(sizeof(uint16_t) * size, roots().code_type());
}

Data *Heap::new_abstract_buffer(uint32_t byte_count, Type *type) {
  int size = AbstractBuffer::size_for(byte_count);
  Data *val = allocate_object(size, type);
  if (is<AllocationFailed>(val)) return val;
  AbstractBuffer *result = cast<AbstractBuffer>(val);
  result->set_size<uint8_t>(byte_count);
  return result;
}

Data *Heap::new_tuple(uint32_t length) {
  int size = Tuple::size_for(length);
  Data *val = allocate_object(size, roots().tuple_type());
  if (is<AllocationFailed>(val)) return val;
  Tuple *result = cast<Tuple>(val);
  result->length() = length;
  for (uint32_t i = 0; i < length; i++)
    result->at(i) = roots().vhoid();
  return result;
}

Data *Heap::new_singleton(Type *type) {
  return allocate_object(Singleton::kSize, type);
}

Data *Heap::new_dictionary() {
  Data *table_val = new_tuple(0);
  if (is<AllocationFailed>(table_val)) return table_val;
  Tuple *table = cast<Tuple>(table_val);
  Data *val = allocate_object(Dictionary::kSize, roots().dictionary_type());
  if (is<AllocationFailed>(val)) return val;
  Dictionary *result = cast<Dictionary>(val);
  result->set_table(table);
  return result;
}
