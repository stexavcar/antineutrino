#include "runtime/heap-inl.h"
#include "value/value-inl.h"
#include "value/condition-inl.h"

namespace neutrino {

allocation<Object> DescriptorDescriptor::clone_object(Object *obj, Space &space) {
  return static_cast<Descriptor*>(obj)->clone(space);
}

void Descriptor::migrate_fields(Object *obj, FieldMigrator &migrator) {
  migrator.migrate_field(&header());
}

/* --- I n s t a n c e --- */

allocation<Object> InstanceDescriptor::clone_object(Object *obj, Space &space) {
  word size = Instance::size_in_memory(field_count());
  array<uint8_t> memory = space.allocate(size);
  if (memory.is_empty()) return InternalError::make(InternalError::ieHeapExhaustion);
  Instance *old_instance = static_cast<Instance*>(obj);
  return new (memory) Instance(*old_instance);
}

word InstanceDescriptor::size_in_memory(Object *obj) {
  return Instance::size_in_memory(field_count());
}

/* --- S t r i n g --- */

allocation<Object> StringDescriptor::clone_object(Object *obj, Space &space) {
  String *old = cast<String>(obj);
  word length = old->length();
  word size = String::size_in_memory(length);
  array<uint8_t> memory = space.allocate(size);
  if (memory.is_empty()) return InternalError::make(InternalError::ieHeapExhaustion);
  String *result = new (memory) String(*old);
  array<code_point> from = old->chars();
  array<code_point> to = result->chars();
  for (word i = 0; i < length; i++)
    to[i] = from[i];
  return result;
}

word StringDescriptor::size_in_memory(Object *obj) {
  return String::size_in_memory(cast<String>(obj)->length());
}

} // namespace neutrino
