#include "heap/roots-inl.h"
#include "heap/values-inl.h"

namespace neutrino {

MAKE_ENUM_INFO_HEADER(RootName)
#define MAKE_ENTRY(n, Type, name, NAME, allocator) MAKE_ENUM_INFO_ENTRY(NAME##_ROOT)
FOR_EACH_ROOT(MAKE_ENTRY)
#undef MAKE_ENTRY
MAKE_ENUM_INFO_FOOTER()

static void clear_field(Value** field, void*) {
  *field = Smi::from_int(0);
}

Roots::Roots() {
  for_each<void*>(clear_field, 0);
}

bool Roots::initialize(Heap& heap) {
  // Complicated roots
  Data *class_class_val = heap.allocate_class(CLASS_TYPE);
  if (is<AllocationFailed>(class_class_val)) return false;
  Class *class_class_obj = reinterpret_cast<Class*>(class_class_val);
  class_class_obj->set_chlass(class_class_obj);
  class_class() = cast<Class>(class_class_obj);
  
  // All the simple roots get allocated the same way, which is what
  // makes them simple.
#define ALLOCATE_ROOT(n, Type, name, NAME, allocator)                \
  Data *name##_val = heap.allocator;                                 \
  if (is<AllocationFailed>(name##_val)) return false;                \
  name() = cast<Type>(name##_val);
FOR_EACH_SIMPLE_ROOT(ALLOCATE_ROOT)
#undef ALLOCATE_ROOT

  class_class()->set_super(vhoid());
  class_class()->set_methods(empty_tuple());

  Data *class_name;
#define SET_CLASS_NAME(n, Type, name, NAME, allocator)               \
  class_name = heap.new_string(#name);                               \
  if (is<AllocationFailed>(class_name)) return false;                \
  name()->set_name(cast<String>(class_name));
FOR_EACH_ROOT_CLASS(SET_CLASS_NAME)
#undef SET_CLASS_NAME
  return true;
}

} // neutrino
