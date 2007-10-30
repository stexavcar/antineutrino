#include "heap/roots-inl.h"
#include "heap/values-inl.h"

using namespace neutrino;

static void clear_field(Value** field, void*) {
  *field = Smi::from_int(0);
}

RootContainer::RootContainer() {
  for_each<void*>(clear_field, 0);
}

bool RootContainer::initialize(Heap& heap) {
  // Complicated roots
  Data *type_type_val = heap.allocate_type(TYPE_TAG);
  if (is<AllocationFailed>(type_type_val)) return false;
  Type *type_type_obj = reinterpret_cast<Type*>(type_type_val);
  type_type_obj->set_type(type_type_obj);
  type_type_ = cast<Type>(type_type_obj);
  
  // All the simple roots get allocated the same way, which is what
  // makes them simple.
#define ALLOCATE_ROOT(Type, name, allocator)                         \
  Data *name##_val = heap.allocator;                                 \
  if (is<AllocationFailed>(name##_val)) return false;                \
  name##_ = cast<Type>(name##_val);
FOR_EACH_SIMPLE_ROOT(ALLOCATE_ROOT)
#undef ALLOCATE_ROOT

  return true;
}
