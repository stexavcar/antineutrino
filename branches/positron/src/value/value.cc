#include "runtime/heap-inl.h"
#include "value/value-inl.h"
#include "value/condition-inl.h"

namespace neutrino {

allocation<Object> DescriptorDescriptor::clone_object(Object *obj, Space &space) {
  try Descriptor *result = static_cast<Descriptor*>(obj)->clone(space);
  return result;
}

allocation<Descriptor> DescriptorDescriptor::clone(Space &space) {
  try alloc {space} DescriptorDescriptor *result(*this);
  return result;
}

allocation<Object> InstanceDescriptor::clone_object(Object *obj, Space &space) {
  word size = sizeof(Instance) + kPointerSize * field_count();
  array<uint8_t> memory = space.allocate(size);
  if (memory.is_empty()) return InternalError::make(InternalError::ieHeapExhaustion);
  Instance *old_instance = static_cast<Instance*>(obj);
  return new (memory) Instance(*old_instance);
}

allocation<Descriptor> InstanceDescriptor::clone(Space &space) {
  try alloc {space} InstanceDescriptor *result(*this);
  return result;
}

} // namespace neutrino
